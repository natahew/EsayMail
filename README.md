# EsayMail
This is a simple Email Wapper

# Example

easy::SmtpMailer smtp(
		"smtp.163.com",
		25,
		"username@163.com",
		"password"
	);

	smtp.AddReceiver("to1@163.com");
	smtp.AddReceiver("to2@163.com");
	smtp.SetTitle("This is a test mail");
	smtp.SetContent("hello,Please do not reply to this email.");

	std::string filePath("./path/testFile.txt");
	smtp.AddAttachment(filePath);

	switch (smtp.SendEmail())
	{
	case easy::ReturnNo::SUCCESS:
		break;
	case easy::ReturnNo::NETWORK_ERROR:
		std::cout << "error 1: NETWORK_ERROR!" << std::endl;
		break;
	case easy::ReturnNo::USERNAME_ERROR:
		std::cout << "error 2: USERNAME_ERROR!" << std::endl;
		break;
	case easy::ReturnNo::PASSWORD_ERROR:
		std::cout << "error 3: PASSWORD_ERROR!" << std::endl;
		break;
	case easy::ReturnNo::FILE_ERROR:
		std::cout << "error 4: FILE_ERROR!" << std::endl;
		break;
	default:
		break;
	}
  
