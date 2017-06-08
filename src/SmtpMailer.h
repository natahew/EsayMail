#ifndef __SMTP_MAILER_H__
#define __SMTP_MAILER_H__

#include <iostream>
#include <fstream>
#include <list>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")                                   //链接ws2_32.lib动态链接库

namespace easy {

	const int kMaxLength = 1024;

	const int kMaxFileLength = 6000;

	static const char kBase64Char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	static const char kBoundary[] = "#==b==o==u==n==d==a==r==y==#";  //boundary

	struct FileInfo
	{
		char fileName[128];                                          //文件名称
		char filePath[256];                                          //文件绝对路径
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
		SmtpMailer(std::string domain,	                             //smtp服务器域名
			int port,                                                //端口
			std::string username,	                                 //用户名
			std::string password,	                                 //密码
			std::string locale = "chs"
		);

		~SmtpMailer();

		/**
		 * 设置服务器域
		 */
		void SetDomain(std::string domain);
		/**
		 * 设置端口
		 */
		void SetEmailPort(int port);
		/**
		 * 设置用户名
		 */
		void SetUserName(std::string user);
		/**
		 * 设置密码
		 */
		void SetPassword(std::string pass);
		/**
		 * 设置邮件标题
		 */
		void SetTitle(std::string title);
		/**
		 * 设置发送文件内容
		 */
		void SetContent(std::string content);
		/**
		 * 设置目的地邮箱
		 */
		void AddReceiver(std::string addr);
		/**
		 * 删除指定目的地邮箱
		 */
		void RemoveReceiver(std::string addr);
		/**
		 * 添加抄送邮箱
		 */
		void AddCCReceiver(std::string addr);
		/**
		 * 删除抄送邮箱
		 */
		void RemoveCCReceiver(std::string addr);
		/**
		 * 添加密送邮箱
		 */
		void AddBCCReceiver(std::string addr);
		/**
		 * 删除密送邮箱
		 */
		void RemoveBCCReceiver(std::string addr);
		/**
		 * 添加附件
		 */
		void AddAttachment(std::string &filePath);
		/**
		 * 删除附件
		 */
		void DeleteAttachment(std::string &filePath);
		/**
		 * 删除所有附件
		 */
		void DeleteAllAttachment();
		/**
		 * 发送邮件
		 * 关于错误码的说明:
		 * 0.成功
		 * 1.网络错误导致的错误
		 * 2.用户名错误
		 * 3.密码错误
		 * 4.文件不存在
		 */
		ReturnNo SendEmail();

	private:
		/**
		 * 创建连接
		 */
		bool CreateConn();
		/**
		 * 登录
		 */
		ReturnNo Login();
		/**
		 * 发送Socket信息
		 */
		bool Send(const std::string &message);
		/**
		 * 接收回报
		 */
		bool Receive();
		/**
		 * 格式化要发送的邮件头部
		 */
		void FormatHeadInfo(std::string &email);
		/**
		 * 发送邮件头部信息
		 */
		bool SendHead();
		/**
		 * 发送文本信息
		 */
		bool SendBody();
		/**
		 * 发送附件
		 */
		ReturnNo SendAttachment();
		/**
		 * 通知内容发送完毕
		 */
		bool SendEndFlag();
		/**
		 * base64加密程序
		 */
		char* Base64Encode(char const* origSigned, std::size_t origLength);
		/**
		 * 发送加密文本
		 */
		ReturnNo SendEncodedStr(const char* plainText, std::size_t len);
		
	private:
		/**
		 * 端口
		 */
		int port_;
		/**
		 * 邮件服务器域名
		 */
		std::string domain_;
		/**
		 * 用户名
		 */
		std::string username_;
		/**
		 * 密码
		 */
		std::string password_;
		/**
		 * 目的地邮箱
		 */
		std::list<std::string> target_email_address_list_;
		/**
		 * 抄送目的地邮箱
		 */
		std::list<std::string> cc_target_email_address_list_;
		/**
		 * 密送目的地邮箱
		 */
		std::list<std::string> bcc_target_email_address_list_;
		/**
		 * 邮件标题
		 */
		std::string title_;
		/**
		 * 是否为纯文本邮件
		 */
		bool is_plain_;
		/**
		 * 邮件正文
		 */
		std::string content_;
		/**
		 * 附件List
		 */
		std::list <FileInfo *> attachment_list_;
		/**
		 * 回报缓冲区
		 */
		char return_buffer_[kMaxLength + 1];
		/**
		 * 客户端套接字
		 */
		SOCKET sock_client_;
	};

}
#endif // !__SMTP_MAILER_H__
