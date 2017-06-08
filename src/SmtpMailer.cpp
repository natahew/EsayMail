#include "SmtpMailer.h"

namespace easy {

	SmtpMailer::~SmtpMailer(void)
	{
		DeleteAllAttachment();
		closesocket(sock_client_);
		WSACleanup();
	}


	SmtpMailer::SmtpMailer(std::string domain,
		int port,
		std::string username,
		std::string password,
		std::string locale)
	{
		setlocale(LC_ALL, locale.c_str());

		domain_ = domain;
		port_ = port;
		username_ = username;
		password_ = password;
		title_ = "";
		content_ = "";
		is_plain_ = false;
		WORD wVersionRequested;
		WSADATA wsaData;
		wVersionRequested = MAKEWORD(2, 1);
		int err;
		err = WSAStartup(wVersionRequested, &wsaData);
		sock_client_ = 0;
	}

	bool SmtpMailer::CreateConn()
	{
		SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
		SOCKADDR_IN addrSrv;
		HOSTENT* pHostent;
		pHostent = gethostbyname(domain_.c_str());

		addrSrv.sin_addr.S_un.S_addr = *((DWORD *)pHostent->h_addr_list[0]);
		addrSrv.sin_family = AF_INET;
		addrSrv.sin_port = htons(port_);
		int err = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
		if (err != 0)
			return false;

		sock_client_ = sockClient;

		if (!Receive())
			return false;

		return true;
	}

	bool SmtpMailer::Send(const std::string &message)
	{
		int err = send(sock_client_, message.c_str(), message.length(), 0);
		if (err == SOCKET_ERROR)
		{
			return false;
		}
		return true;
	}

	ReturnNo SmtpMailer::SendEncodedStr(const char * plainText, std::size_t len)
	{
		ReturnNo stat = ReturnNo::SUCCESS;
		char *chSendBuff = Base64Encode(plainText, len);
		std::size_t encodeLength = strlen(chSendBuff);
		chSendBuff[encodeLength] = '\r';
		chSendBuff[encodeLength + 1] = '\n';
		chSendBuff[encodeLength + 2] = '\0';
		int err = send(sock_client_, chSendBuff, strlen(chSendBuff), 0);
		if (err != strlen(chSendBuff))
		{
			stat = ReturnNo::NETWORK_ERROR;
		}
		delete[] chSendBuff;
		return stat;
	}

	bool SmtpMailer::Receive()
	{
		memset(return_buffer_, 0, sizeof(char)* (kMaxLength + 1));
		int err = recv(sock_client_, return_buffer_, kMaxLength, 0); //接收数据
		if (err == SOCKET_ERROR)
			return false;

		return_buffer_[err] = '\0';
		std::cout << return_buffer_ << std::endl;
		return true;
	}

	ReturnNo SmtpMailer::Login()
	{
		std::string sendBuff;
		sendBuff = "EHLO\r\n";

		if (!Send(sendBuff) || !Receive())
		{
			return ReturnNo::NETWORK_ERROR;
		}

		sendBuff = "AUTH LOGIN\r\n";
		if (!Send(sendBuff) || !Receive())
		{
			return ReturnNo::NETWORK_ERROR;
		}

		std::size_t pos = username_.find('@', 0);
		sendBuff = username_.substr(0, pos);

		char* ecode = nullptr;

		ecode = Base64Encode(sendBuff.c_str(), strlen(sendBuff.c_str()));
		sendBuff = ecode;
		sendBuff += "\r\n";
		delete[]ecode;

		if (!Send(sendBuff) || !Receive())
		{
			return ReturnNo::NETWORK_ERROR;
		}

		ecode = Base64Encode(password_.c_str(), strlen(password_.c_str()));
		sendBuff = ecode;
		sendBuff += "\r\n";
		delete[]ecode;

		if (!Send(sendBuff) || !Receive())
		{
			return ReturnNo::NETWORK_ERROR;
		}

		if (strstr(return_buffer_, "550") != nullptr)
		{
			return ReturnNo::USERNAME_ERROR;
		}

		if (strstr(return_buffer_, "535") != nullptr)
		{
			return ReturnNo::PASSWORD_ERROR;
		}
		return ReturnNo::SUCCESS;
	}

	bool SmtpMailer::SendHead()
	{
		std::string sendBuff;
		sendBuff = "MAIL FROM: <" + username_ + ">\r\n";
		if (!Send(sendBuff) || !Receive())
		{
			return false;
		}

		for (auto& address : target_email_address_list_)
		{
			sendBuff = "RCPT TO: <" + address + ">\r\n";
			if (!Send(sendBuff) || !Receive())
			{
				return false;
			}
		}

		for (auto& address : cc_target_email_address_list_)
		{
			sendBuff = "RCPT TO: <" + address + ">\r\n";
			if (!Send(sendBuff) || !Receive())
			{
				return false;
			}
		}

		for (auto& address : bcc_target_email_address_list_)
		{
			sendBuff = "RCPT TO: <" + address + ">\r\n";
			if (!Send(sendBuff) || !Receive())
			{
				return false;
			}
		}

		sendBuff = "DATA\r\n";
		if (!Send(sendBuff) || !Receive())
		{
			return false;
		}

		FormatHeadInfo(sendBuff);

		//发送完头部之后不必调用接收函数,因为你没有\r\n.\r\n结尾，
		//服务器认为你没有发完数据，所以不会返回什么值
		if (!Send(sendBuff))
		{
			return false;
		}
		return true;
	}

	void SmtpMailer::FormatHeadInfo(std::string &email)
	{
		//设置发件人
		email = "From: ";
		email += username_;
		email += "\r\n";
		//设置收件人
		email += "To:";
		for (auto& address : target_email_address_list_)
		{
			email += "<";
			email += address;
			email += ">;";
		}
		email += "\r\n";

		//设置抄送
		if (cc_target_email_address_list_.size() > 0) {
			email += "CC:";
			for (auto& address : cc_target_email_address_list_)
			{
				email += "<";
				email += address;
				email += ">;";
			}
			email += "\r\n";
		}

		//设置密送
		if (bcc_target_email_address_list_.size() > 0) {
			email += "BCC:";
			for (auto& address : bcc_target_email_address_list_)
			{
				email += "<";
				email += address;
				email += ">;";
			}
			email += "\r\n";
		}

		email += "Subject: ";
		email += title_;
		email += "\r\n";

		email += "MIME-Version: 1.0";
		email += "\r\n";

		email += "Content-Type: multipart/mixed;boundary=";
		email += kBoundary;
		email += "\r\n";
		email += "\r\n";
	}

	bool SmtpMailer::SendBody()
	{
		std::string sendBuff;
		sendBuff = "--";
		sendBuff += kBoundary;
		sendBuff += "\r\n";
		sendBuff += "Content-Type: text/plain;";
		sendBuff += "charset=\"gb2312\"\r\n\r\n";
		sendBuff += content_;
		sendBuff += "\r\n\r\n";
		return Send(sendBuff);
	}

	ReturnNo SmtpMailer::SendAttachment()
	{
		for (auto& pIter : attachment_list_)
		{
			std::cout << "Attachment is sending ~~~~~" << std::endl;
			std::cout << "Please be patient!" << std::endl;
			std::string sendBuff;
			sendBuff = "--";
			sendBuff += kBoundary;
			sendBuff += "\r\n";
			sendBuff += "Content-Type: application/octet-stream;\r\n";
			sendBuff += " name=\"";
			sendBuff += pIter->fileName;
			sendBuff += "\"";
			sendBuff += "\r\n";

			sendBuff += "Content-Transfer-Encoding: base64\r\n";
			sendBuff += "Content-Disposition: attachment;\r\n";
			sendBuff += " filename=\"";
			sendBuff += pIter->fileName;
			sendBuff += "\"";

			sendBuff += "\r\n";
			sendBuff += "\r\n";
			Send(sendBuff);
			std::ifstream ifs(pIter->filePath, std::ios::in | std::ios::binary);
			if (!ifs.is_open())
			{
				return ReturnNo::FILE_ERROR;
			}

			char fileBuff[kMaxFileLength];
			memset(fileBuff, '\0', sizeof(fileBuff));
			//文件使用base64加密传送
			while (ifs.read(fileBuff, kMaxFileLength))
			{
				ReturnNo stat = SendEncodedStr(fileBuff, kMaxFileLength);
				if (stat != ReturnNo::SUCCESS)
					return stat;
			}
			//处理余量
			ReturnNo stat = SendEncodedStr(fileBuff, ifs.gcount());
			if (stat != ReturnNo::SUCCESS)
				return stat;
		}
		return ReturnNo::SUCCESS;
	}

	bool SmtpMailer::SendEndFlag()
	{
		std::string sendBuff;
		sendBuff = "--";
		sendBuff += kBoundary;
		sendBuff += "--";
		sendBuff += "\r\n.\r\n";
		if (!Send(sendBuff) || !Receive())
		{
			return false;
		}
		sendBuff = "QUIT\r\n";
		return (Send(sendBuff) && Receive());
	}

	ReturnNo SmtpMailer::SendEmail()
	{
		if (!CreateConn())
		{
			return ReturnNo::NETWORK_ERROR;
		}

		ReturnNo err = Login();
		if (err != ReturnNo::SUCCESS)
		{
			return err;
		}

		if (!SendHead())
		{
			return ReturnNo::NETWORK_ERROR;
		}

		if (!SendBody())
		{
			return ReturnNo::NETWORK_ERROR;
		}

		err = SendAttachment();
		if (err != ReturnNo::SUCCESS)
		{
			return err;
		}

		if (!SendEndFlag())
		{
			return ReturnNo::NETWORK_ERROR;
		}

		return ReturnNo::SUCCESS;
	}

	void SmtpMailer::AddAttachment(std::string &filePath)
	{
		FileInfo *pFile = new FileInfo;
		std::strncpy(pFile->filePath, filePath.c_str(), sizeof(pFile->filePath));
		const char *p = filePath.c_str();
		std::strncpy(pFile->fileName, p + filePath.find_last_of("\\") + 1,
			sizeof(pFile->fileName));
		attachment_list_.push_back(pFile);
	}

	void SmtpMailer::DeleteAttachment(std::string &filePath)
	{
		std::list<FileInfo *>::iterator pIter;
		for (pIter = attachment_list_.begin(); pIter != attachment_list_.end(); pIter++)
		{
			if (strcmp((*pIter)->filePath, filePath.c_str()) == 0)
			{
				FileInfo *p = *pIter;
				attachment_list_.remove(*pIter);
				delete p;
				break;
			}
		}
	}

	void SmtpMailer::DeleteAllAttachment()
	{
		std::list<FileInfo *>::iterator pIter;
		for (pIter = attachment_list_.begin(); pIter != attachment_list_.end();)
		{
			FileInfo *p = *pIter;
			pIter = attachment_list_.erase(pIter);
			delete p;
		}
	}

	void SmtpMailer::SetDomain(std::string domain)
	{
		domain_ = domain;
	}

	void SmtpMailer::SetUserName(std::string user)
	{
		username_ = user;
	}

	void SmtpMailer::SetPassword(std::string pass)
	{
		password_ = pass;
	}

	void SmtpMailer::AddReceiver(std::string addr)
	{
		target_email_address_list_.emplace_back(addr);
	}

	void SmtpMailer::RemoveReceiver(std::string addr)
	{
		target_email_address_list_.remove(addr);
	}

	void SmtpMailer::AddCCReceiver(std::string addr)
	{
		cc_target_email_address_list_.emplace_back(addr);
	}

	void SmtpMailer::RemoveCCReceiver(std::string addr)
	{
		cc_target_email_address_list_.remove(addr);
	}

	void SmtpMailer::AddBCCReceiver(std::string addr)
	{
		bcc_target_email_address_list_.emplace_back(addr);
	}

	void SmtpMailer::RemoveBCCReceiver(std::string addr)
	{
		bcc_target_email_address_list_.remove(addr);
	}

	void SmtpMailer::SetTitle(std::string title)
	{
		title_ = title;
	}

	void SmtpMailer::SetContent(std::string content)
	{
		content_ = content;
	}

	void SmtpMailer::SetEmailPort(int port)
	{
		port_ = port;
	}

	char* SmtpMailer::Base64Encode(char const* origSigned, std::size_t origLength)
	{
		unsigned char const* orig = (unsigned char const*)origSigned; // in case any input bytes have the MSB set
		if (orig == nullptr)
			return nullptr;

		std::size_t numOrig24BitValues = origLength / 3;
		bool havePadding = origLength > numOrig24BitValues * 3;
		bool havePadding2 = (origLength == numOrig24BitValues * 3 + 2);
		std::size_t numResultBytes = 4 * (numOrig24BitValues + havePadding);
		char* result = new char[numResultBytes + 3]; // allow for trailing '/0'
		// Map each full group of 3 input bytes into 4 output base-64 characters:
		unsigned i;
		for (i = 0; i < numOrig24BitValues; ++i)
		{
			result[4 * i + 0] = kBase64Char[(orig[3 * i] >> 2) & 0x3F];
			result[4 * i + 1] = kBase64Char[(((orig[3 * i] & 0x3) << 4) | (orig[3 * i + 1] >> 4)) & 0x3F];
			result[4 * i + 2] = kBase64Char[((orig[3 * i + 1] << 2) | (orig[3 * i + 2] >> 6)) & 0x3F];
			result[4 * i + 3] = kBase64Char[orig[3 * i + 2] & 0x3F];
		}

		// Now, take padding into account.  (Note: i == numOrig24BitValues)
		if (havePadding)
		{
			result[4 * i + 0] = kBase64Char[(orig[3 * i] >> 2) & 0x3F];
			if (havePadding2)
			{
				result[4 * i + 1] = kBase64Char[(((orig[3 * i] & 0x3) << 4) | (orig[3 * i + 1] >> 4)) & 0x3F];
				result[4 * i + 2] = kBase64Char[(orig[3 * i + 1] << 2) & 0x3F];
			}
			else
			{
				result[4 * i + 1] = kBase64Char[((orig[3 * i] & 0x3) << 4) & 0x3F];
				result[4 * i + 2] = '=';
			}
			result[4 * i + 3] = '=';
		}
		result[numResultBytes] = '\0';
		return result;
	}
}