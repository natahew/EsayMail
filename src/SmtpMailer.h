#ifndef __SMTP_MAILER_H__
#define __SMTP_MAILER_H__

#include <iostream>
#include <fstream>
#include <list>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")                                   //����ws2_32.lib��̬���ӿ�

namespace easy {

	const int kMaxLength = 1024;

	const int kMaxFileLength = 6000;

	static const char kBase64Char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	static const char kBoundary[] = "#==b==o==u==n==d==a==r==y==#";  //boundary

	struct FileInfo
	{
		char fileName[128];                                          //�ļ�����
		char filePath[256];                                          //�ļ�����·��
	};

	enum class ReturnNo
	{
		SUCCESS = 0,
		NETWORK_ERROR = 1,
		USERNAME_ERROR = 2,
		PASSWORD_ERROR = 3,
		FILE_ERROR = 4,
	};

	class SmtpMailer
	{
	public:
		SmtpMailer(std::string domain,	                             //smtp����������
			int port,                                                //�˿�
			std::string username,	                                 //�û���
			std::string password,	                                 //����
			std::string locale = "chs"
		);

		~SmtpMailer();

		/**
		 * ���÷�������
		 */
		void SetDomain(std::string domain);
		/**
		 * ���ö˿�
		 */
		void SetEmailPort(int port);
		/**
		 * �����û���
		 */
		void SetUserName(std::string user);
		/**
		 * ��������
		 */
		void SetPassword(std::string pass);
		/**
		 * �����ʼ�����
		 */
		void SetTitle(std::string title);
		/**
		 * ���÷����ļ�����
		 */
		void SetContent(std::string content);
		/**
		 * ����Ŀ�ĵ�����
		 */
		void AddReceiver(std::string addr);
		/**
		 * ɾ��ָ��Ŀ�ĵ�����
		 */
		void RemoveReceiver(std::string addr);
		/**
		 * ��ӳ�������
		 */
		void AddCCReceiver(std::string addr);
		/**
		 * ɾ����������
		 */
		void RemoveCCReceiver(std::string addr);
		/**
		 * �����������
		 */
		void AddBCCReceiver(std::string addr);
		/**
		 * ɾ����������
		 */
		void RemoveBCCReceiver(std::string addr);
		/**
		 * ��Ӹ���
		 */
		void AddAttachment(std::string &filePath);
		/**
		 * ɾ������
		 */
		void DeleteAttachment(std::string &filePath);
		/**
		 * ɾ�����и���
		 */
		void DeleteAllAttachment();
		/**
		 * �����ʼ�
		 * ���ڴ������˵��:
		 * 0.�ɹ�
		 * 1.��������µĴ���
		 * 2.�û�������
		 * 3.�������
		 * 4.�ļ�������
		 */
		ReturnNo SendEmail();

	private:
		/**
		 * ��������
		 */
		bool CreateConn();
		/**
		 * ��¼
		 */
		ReturnNo Login();
		/**
		 * ����Socket��Ϣ
		 */
		bool Send(const std::string &message);
		/**
		 * ���ջر�
		 */
		bool Receive();
		/**
		 * ��ʽ��Ҫ���͵��ʼ�ͷ��
		 */
		void FormatHeadInfo(std::string &email);
		/**
		 * �����ʼ�ͷ����Ϣ
		 */
		bool SendHead();
		/**
		 * �����ı���Ϣ
		 */
		bool SendBody();
		/**
		 * ���͸���
		 */
		ReturnNo SendAttachment();
		/**
		 * ֪ͨ���ݷ������
		 */
		bool SendEndFlag();
		/**
		 * base64���ܳ���
		 */
		char* Base64Encode(char const* origSigned, std::size_t origLength);
		/**
		 * ���ͼ����ı�
		 */
		ReturnNo SendEncodedStr(const char* plainText, std::size_t len);
		
	private:
		/**
		 * �˿�
		 */
		int port_;
		/**
		 * �ʼ�����������
		 */
		std::string domain_;
		/**
		 * �û���
		 */
		std::string username_;
		/**
		 * ����
		 */
		std::string password_;
		/**
		 * Ŀ�ĵ�����
		 */
		std::list<std::string> target_email_address_list_;
		/**
		 * ����Ŀ�ĵ�����
		 */
		std::list<std::string> cc_target_email_address_list_;
		/**
		 * ����Ŀ�ĵ�����
		 */
		std::list<std::string> bcc_target_email_address_list_;
		/**
		 * �ʼ�����
		 */
		std::string title_;
		/**
		 * �Ƿ�Ϊ���ı��ʼ�
		 */
		bool is_plain_;
		/**
		 * �ʼ�����
		 */
		std::string content_;
		/**
		 * ����List
		 */
		std::list <FileInfo *> attachment_list_;
		/**
		 * �ر�������
		 */
		char return_buffer_[kMaxLength + 1];
		/**
		 * �ͻ����׽���
		 */
		SOCKET sock_client_;
	};

}
#endif // !__SMTP_MAILER_H__
