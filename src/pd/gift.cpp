#include "gift.h"

// --- check that src image is actually image, resize it and save to dst
static bool ImageSaveAsJpgToFeedFolder (const string src, const string dst, struct ExifInfo &exifInfo, c_config *config)
{
	{
		CLog	log;
		ostringstream   ost;

		ost.str("");
		ost << "[" << __LINE__ << "]:" << __func__ << " (" << src << ", " << dst << "): enter";
		log.Write(DEBUG, ost.str());
	}

#ifndef IMAGEMAGICK_DISABLE
	// Construct the image object. Separating image construction FROM the
	// the read operation ensures that a failure to read the image file
	// doesn't render the image object useless.
	try {
		Magick::Image		   image;
		Magick::OrientationType imageOrientation;
		Magick::Geometry		imageGeometry;

		auto	feed_image_max_width = stod_noexcept(config->GetFromFile("image_max_width", "gift"));
		auto	feed_image_max_height = stod_noexcept(config->GetFromFile("image_max_height", "gift"));

		// Read a file into image object
		image.read( src );  /* Flawfinder: ignore */

		imageGeometry = image.size();
		imageOrientation = image.orientation();

		{
			CLog	log;
			ostringstream   ost;

			ost.str("");
			ost << "[" << __LINE__ << "]:" << __func__ << " (" << src << ", " << dst << "): imageOrientation = " << imageOrientation << ", xRes = " << imageGeometry.width() << ", yRes = " << imageGeometry.height();
			log.Write(DEBUG, ost.str());
		}

		if(imageOrientation == Magick::TopRightOrientation) image.flop();
		if(imageOrientation == Magick::BottomRightOrientation) image.rotate(180);
		if(imageOrientation == Magick::BottomLeftOrientation) { image.flop(); image.rotate(180); }
		if(imageOrientation == Magick::LeftTopOrientation) { image.flop(); image.rotate(-90); }
		if(imageOrientation == Magick::RightTopOrientation) image.rotate(90);
		if(imageOrientation == Magick::RightBottomOrientation) { image.flop(); image.rotate(90); }
		if(imageOrientation == Magick::LeftBottomOrientation) image.rotate(-90);

		if((imageGeometry.width() > feed_image_max_width) || (imageGeometry.height() > feed_image_max_height))
		{
			int   newHeight, newWidth;
			if(imageGeometry.width() >= imageGeometry.height())
			{
				newWidth = feed_image_max_width;
				newHeight = newWidth * imageGeometry.height() / imageGeometry.width();
			}
			else
			{
				newHeight = feed_image_max_height;
				newWidth = newHeight * imageGeometry.width() / imageGeometry.height();
			}

			image.resize(Magick::Geometry(newWidth, newHeight, 0, 0));
		}

		{
			// --- Exif information save
			exifInfo.DateTime = "";
			exifInfo.DateTime = image.attribute("exif:DateTime");
			exifInfo.GPSAltitude = "";
			exifInfo.GPSAltitude = image.attribute("exif:GPSAltitude");
			exifInfo.GPSLatitude = "";
			exifInfo.GPSLatitude = image.attribute("exif:GPSLatitude");
			exifInfo.GPSLongitude = "";
			exifInfo.GPSLongitude = image.attribute("exif:GPSLongitude");
			exifInfo.GPSSpeed = "";
			exifInfo.GPSSpeed = image.attribute("exif:GPSSpeed");
			exifInfo.Model = "";
			exifInfo.Model = image.attribute("exif:Model");
			exifInfo.Authors = "";
			exifInfo.Authors = image.attribute("exif:Authors");
			exifInfo.ApertureValue = "";
			exifInfo.ApertureValue = image.attribute("exif:ApertureValue");
			exifInfo.BrightnessValue = "";
			exifInfo.BrightnessValue = image.attribute("exif:BrightnessValue");
			exifInfo.ColorSpace = "";
			exifInfo.ColorSpace = image.attribute("exif:ColorSpace");
			exifInfo.ComponentsConfiguration = "";
			exifInfo.ComponentsConfiguration = image.attribute("exif:ComponentsConfiguration");
			exifInfo.Compression = "";
			exifInfo.Compression = image.attribute("exif:Compression");
			exifInfo.DateTimeDigitized = "";
			exifInfo.DateTimeDigitized = image.attribute("exif:DateTimeDigitized");
			exifInfo.DateTimeOriginal = "";
			exifInfo.DateTimeOriginal = image.attribute("exif:DateTimeOriginal");
			exifInfo.ExifImageLength = "";
			exifInfo.ExifImageLength = image.attribute("exif:ExifImageLength");
			exifInfo.ExifImageWidth = "";
			exifInfo.ExifImageWidth = image.attribute("exif:ExifImageWidth");
			exifInfo.ExifOffset = "";
			exifInfo.ExifOffset = image.attribute("exif:ExifOffset");
			exifInfo.ExifVersion = "";
			exifInfo.ExifVersion = image.attribute("exif:ExifVersion");
			exifInfo.ExposureBiasValue = "";
			exifInfo.ExposureBiasValue = image.attribute("exif:ExposureBiasValue");
			exifInfo.ExposureMode = "";
			exifInfo.ExposureMode = image.attribute("exif:ExposureMode");
			exifInfo.ExposureProgram = "";
			exifInfo.ExposureProgram = image.attribute("exif:ExposureProgram");
			exifInfo.ExposureTime = "";
			exifInfo.ExposureTime = image.attribute("exif:ExposureTime");
			exifInfo.Flash = "";
			exifInfo.Flash = image.attribute("exif:Flash");
			exifInfo.FlashPixVersion = "";
			exifInfo.FlashPixVersion = image.attribute("exif:FlashPixVersion");
			exifInfo.FNumber = "";
			exifInfo.FNumber = image.attribute("exif:FNumber");
			exifInfo.FocalLength = "";
			exifInfo.FocalLength = image.attribute("exif:FocalLength");
			exifInfo.FocalLengthIn35mmFilm = "";
			exifInfo.FocalLengthIn35mmFilm = image.attribute("exif:FocalLengthIn35mmFilm");
			exifInfo.GPSDateStamp = "";
			exifInfo.GPSDateStamp = image.attribute("exif:GPSDateStamp");
			exifInfo.GPSDestBearing = "";
			exifInfo.GPSDestBearing = image.attribute("exif:GPSDestBearing");
			exifInfo.GPSDestBearingRef = "";
			exifInfo.GPSDestBearingRef = image.attribute("exif:GPSDestBearingRef");
			exifInfo.GPSImgDirection = "";
			exifInfo.GPSImgDirection = image.attribute("exif:GPSImgDirection");
			exifInfo.GPSImgDirectionRef = "";
			exifInfo.GPSImgDirectionRef = image.attribute("exif:GPSImgDirectionRef");
			exifInfo.GPSInfo = "";
			exifInfo.GPSInfo = image.attribute("exif:GPSInfo");
			exifInfo.GPSTimeStamp = "";
			exifInfo.GPSTimeStamp = image.attribute("exif:GPSTimeStamp");
			exifInfo.ISOSpeedRatings = "";
			exifInfo.ISOSpeedRatings = image.attribute("exif:ISOSpeedRatings");
			exifInfo.JPEGInterchangeFormat = "";
			exifInfo.JPEGInterchangeFormat = image.attribute("exif:JPEGInterchangeFormat");
			exifInfo.JPEGInterchangeFormatLength = "";
			exifInfo.JPEGInterchangeFormatLength = image.attribute("exif:JPEGInterchangeFormatLength");
			exifInfo.Make = "";
			exifInfo.Make = image.attribute("exif:Make");
			exifInfo.MeteringMode = "";
			exifInfo.MeteringMode = image.attribute("exif:MeteringMode");
			exifInfo.Orientation = "";
			exifInfo.Orientation = image.attribute("exif:Orientation");
			exifInfo.ResolutionUnit = "";
			exifInfo.ResolutionUnit = image.attribute("exif:ResolutionUnit");
			exifInfo.SceneCaptureType = "";
			exifInfo.SceneCaptureType = image.attribute("exif:SceneCaptureType");
			exifInfo.SceneType = "";
			exifInfo.SceneType = image.attribute("exif:SceneType");
			exifInfo.SensingMethod = "";
			exifInfo.SensingMethod = image.attribute("exif:SensingMethod");
			exifInfo.ShutterSpeedValue = "";
			exifInfo.ShutterSpeedValue = image.attribute("exif:ShutterSpeedValue");
			exifInfo.Software = "";
			exifInfo.Software = image.attribute("exif:Software");
			exifInfo.SubjectArea = "";
			exifInfo.SubjectArea = image.attribute("exif:SubjectArea");
			exifInfo.SubSecTimeDigitized = "";
			exifInfo.SubSecTimeDigitized = image.attribute("exif:SubSecTimeDigitized");
			exifInfo.SubSecTimeOriginal = "";
			exifInfo.SubSecTimeOriginal = image.attribute("exif:SubSecTimeOriginal");
			exifInfo.WhiteBalance = "";
			exifInfo.WhiteBalance = image.attribute("exif:WhiteBalance");
			exifInfo.XResolution = "";
			exifInfo.XResolution = image.attribute("exif:XResolution");
			exifInfo.YCbCrPositioning = "";
			exifInfo.YCbCrPositioning = image.attribute("exif:YCbCrPositioning");
			exifInfo.YResolution = "";
			exifInfo.YResolution = image.attribute("exif:YResolution");

			exifInfo.GPSAltitude = image.attribute("exif:GPSAltitudeRef") + ": " + exifInfo.GPSAltitude;
			exifInfo.GPSLatitude = image.attribute("exif:GPSLatitudeRef") + ": " + exifInfo.GPSLatitude;
			exifInfo.GPSLongitude = image.attribute("exif:GPSLongitudeRef") + ": " + exifInfo.GPSLongitude;
			exifInfo.GPSSpeed = image.attribute("exif:GPSSpeedRef") + ": " + exifInfo.GPSSpeed;

			image.strip();
		}


		// Write the image to a file
		image.write( dst );
	}
	catch( Magick::Exception &error_ )
	{
		{
			CLog	log;
			ostringstream   ost;

			ost.str("");
			ost << "[" << __LINE__ << "]:" << __func__ << " (" << src << ", " << dst << "): exception in read/write operation [" << error_.what() << "]";
			log.Write(DEBUG, ost.str());
		}
		return false;
	}
	{
		CLog	log;
		ostringstream   ost;

		ost.str("");
		ost << "[" << __LINE__ << "]:" << __func__ << " (" << src << ", " << dst << "): image has been successfully converted to .jpg format";
		log.Write(DEBUG, ost.str());
	}
	return true;
#else
	{
		CLog	log;
		ostringstream   ost;

		ost.str("");
		ost << "[" << __LINE__ << "]:" << __func__ << " (" << src << ", " << dst << "): simple file coping cause ImageMagick++ is not activated";
		log.Write(DEBUG, ost.str());
	}
	CopyFile(src, dst);
	return  true;
#endif
}


int main()
{
	CStatistics		appStat;  // --- CStatistics must be first statement to measure end2end param's
	CCgi			indexPage(EXTERNAL_TEMPLATE);
	c_config		config(CONFIG_DIR);
	CUser			user;
	string			action, partnerID;
	CMysql			db;
	struct timeval	tv;

	MESSAGE_DEBUG("", "", __FILE__);

	signal(SIGSEGV, crash_handler); 

	gettimeofday(&tv, NULL);
	srand(tv.tv_sec * tv.tv_usec * 100000);  /* Flawfinder: ignore */

	try
	{

	indexPage.ParseURL();

	if(!indexPage.SetTemplate("index.htmlt"))
	{
		CLog	log;

		log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: template file was missing");
		throw CException("Template file was missing");
	}

	if(db.Connect(&config) < 0)
	{
		CLog	log;

		log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: Can not connect to mysql database");
		throw CException("MySql connection");
	}

	indexPage.SetDB(&db);

	action = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("action"));
	MESSAGE_DEBUG("", "", "action taken from HTTP is " + action);

	// --- generate common parts
	{
		// --- it has to be run before session, because session relay upon Apache environment variable
		if(RegisterInitialVariables(&indexPage, &db, &user)) {}
		else
		{
			MESSAGE_ERROR("", "", "RegisterInitialVariables failed, throwing exception");
			throw CExceptionHTML("environment variable error");
		}

		//------- Generate session
		action = GenerateSession(action, &config, &indexPage, &db, &user);
	}
	// ------------ end generate common parts

	MESSAGE_DEBUG("", "", "pre-condition if(action == \"" + action + "\")");

	if(action == "AJAX_addGift")
	{
		ostringstream   ostResult;
		string		    loginFromUser;

		MESSAGE_DEBUG("", action, "start");

		ostResult.str("");
		if(user.GetLogin() == "Guest")
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": re-login required");
			}

			ostResult << "{\"result\":\"error\",\"description\":\"re-login required\"}";
		}
		else
		{
			string	title = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("title"));
			string	link = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("link"));
			string	price = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("price"));
			string	quantity = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("quantity"));
			string	description = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("description"));

			if(title.empty())
			{
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": title should not be empty");
				}

				ostResult << "{\"result\":\"error\",\"description\":\"укажите название подарка\"}";
			}
			else
			{
				unsigned long 	giftID = 0;

				giftID = db.InsertQuery("INSERT INTO `gifts` SET "
										"`title` = \"" + title + "\", "
										"`link` = \"" + link + "\", "
										+ (price.length() ? "`estimated_price` = \"" + price + "\", " : "")
										+ (quantity.length() ? "`requested_quantity` = \"" + quantity + "\", " : "") +
										"`description` = \"" + description + "\", "
										"`user_id` = \"" + user.GetID() + "\", "
										"`eventTimestamp` = UNIX_TIMESTAMP();"
										);

				if(giftID)
				{

					ostResult << "{\"result\":\"success\",\"newGiftID\":\"" << giftID << "\",\"gifts\":[" + GetGiftListInJSONFormat("SELECT * FROM `gifts` WHERE `user_id`=\"" + user.GetID() + "\";", &db, &user) + "]}";
				}
				else
				{
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ":ERROR: insertion into `gift` table");
					}
					ostResult << "{\"result\":\"error\",\"description\":\"ошибка БД\"}";
				}
			}
		}

		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file json_response.htmlt was missing");
			throw CException("Template file json_response.htmlt was missing");
		}  // if(!indexPage.SetTemplate("AJAX_addGift.htmlt"))

		MESSAGE_DEBUG("", action, "finish");
	}

	if((action == "updateGiftTitle") || (action == "updateGiftPrice") || (action == "updateGiftRequestedQuantity") || (action == "updateGiftLink") || (action == "updateGiftDescription"))
	{
		MESSAGE_DEBUG("", action, "start");

		auto	success_message = ""s;
		auto	error_message = ""s;

		ostringstream   ostResult;

		ostResult.str("");
		if(user.GetLogin() == "Guest")
		{
			error_message = gettext("you are not authorized");
			MESSAGE_DEBUG("", action, "start");
		}
		else
		{
			auto	giftID = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));
			auto	value = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value"));

			if(giftID.empty())
			{
				error_message = gettext("mandatory parameter missed");
				MESSAGE_DEBUG("", action, "start");
			}
			else
			{
				if(db.Query("SELECT * FROM `gifts` WHERE `id`=\"" + giftID + "\" AND `user_id`=\"" + user.GetID() + "\";"))
				{
					if(action == "updateGiftTitle")
						db.Query("UPDATE `gifts` SET `title`=\"" + value + "\" WHERE `id`=\"" + giftID + "\";");
					if(action == "updateGiftPrice")
						db.Query("UPDATE `gifts` SET `estimated_price`=\"" + value + "\" WHERE `id`=\"" + giftID + "\";");
					if(action == "updateGiftLink")
						db.Query("UPDATE `gifts` SET `link`=\"" + value + "\" WHERE `id`=\"" + giftID + "\";");
					if(action == "updateGiftDescription")
						db.Query("UPDATE `gifts` SET `description`=\"" + value + "\" WHERE `id`=\"" + giftID + "\";");
					if(action == "updateGiftRequestedQuantity")
					{
						if(value.length() <= 11)
						{
							auto	requested_quantity_new = stod_noexcept(value);
							auto	gained_quantity = stod_noexcept(db.Get(0, "gained_quantity"));

							if(requested_quantity_new >= gained_quantity)
							{
								db.Query("UPDATE `gifts` SET `requested_quantity`=\"" + value + "\" WHERE `id`=\"" + giftID + "\";");

								if(db.isError())
								{
									error_message = gettext("SQL syntax error");
									MESSAGE_ERROR("", action, "start");
								}
								else
								{
									success_message = "\"gifts\":[" + GetGiftListInJSONFormat("SELECT * FROM `gifts` WHERE `user_id`=\"" + user.GetID() + "\";", &db, &user) + "]";
								}
							}
							else
							{
								error_message = "Вам уже подарили больше чем нужно";
								MESSAGE_DEBUG("", "", error_message);
							}
						}
						else
						{
							error_message = "слишком длинное число";
							MESSAGE_ERROR("", "", error_message);
						}
					}

				}
				else
				{
					error_message = gettext("gift not found");
					MESSAGE_ERROR("", "", error_message);
				}
			}
		}

		AJAX_ResponseTemplate(&indexPage, success_message, error_message);

		MESSAGE_DEBUG("", action, "finish");
	}   // if(action == "updateGiftTitle")

	if((action == "AJAX_giveGiftVisible") || (action == "AJAX_giveGiftAnonymous"))
	{
		ostringstream   ostResult;
		string		    loginFromUser;

		MESSAGE_DEBUG("", action, "start");

		if(user.GetLogin() == "Guest")
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": re-login required");
			}

			ostResult << "{\"result\":\"error\",\"description\":\"re-login required\"}";
		}
		else
		{
			string	gift_id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));

			if(db.Query("SELECT * FROM `gifts` WHERE `id`=\"" + gift_id + "\";"))
			{
				string			giftOwnerID = db.Get(0, "user_id");
				string			gained_quantity = db.Get(0, "gained_quantity");
				string			requested_quantity = db.Get(0, "requested_quantity");

				if(
					(user.GetLogin() == "Guest") ||
					(db.Query("SELECT * FROM `gifts_to_give` WHERE `user_id`=\"" + user.GetID() + "\" AND `gift_id`=\"" + gift_id + "\";") == 0)
				  )
				{

					string			visibility = "anonymous";
					unsigned long	reservedToGive = 0;

					if((user.GetLogin() != "Guest") && (action == "AJAX_giveGiftVisible"))
						visibility = "visible";

					if(db.Query("SELECT COUNT(*) as count FROM `gifts_to_give` WHERE `gift_id`=\"" + gift_id + "\";"))
						reservedToGive = stol(db.Get(0, "count"));

					if(stoul(requested_quantity) > (stoul(gained_quantity) + reservedToGive))
					{
						unsigned long 	giftToGiveID = 0;

						giftToGiveID = db.InsertQuery("INSERT INTO `gifts_to_give` SET "
												"`user_id` = \"" + user.GetID() + "\", "
												"`gift_id` = \"" + gift_id + "\", "
												"`visibility` = \"" + visibility + "\", "
												"`eventTimestamp` = UNIX_TIMESTAMP();"
												);
						if(giftToGiveID)
						{
							ostResult << "{"
											"\"result\":\"success\","
											"\"status\":\"reserved\","
											"\"description\":\"зарезервировано на 30 дней\","
											"\"recipient_user_id\":\"" + giftOwnerID + "\","
											"\"gifts_to_give\": [" << GetGiftToGiveListInJSONFormat("SELECT * FROM `gifts_to_give` WHERE `gift_id` in (SELECT `id` FROM `gifts` WHERE `user_id`=\"" + giftOwnerID + "\");", &db, &user) << "]"
											"}";
						}
						else
						{
							{
								CLog	log;
								log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ":ERROR: insertion into `gift` table");
							}
							ostResult << "{\"result\":\"error\",\"description\":\"ошибка БД\"}";
						}
					}
					else
					{
						{
							CLog	log;
							log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": user(" + giftOwnerID + ") doesn't need so many instances of this gift(" + gift_id + ")");
						}
						ostResult << "{\"result\":\"error\",\"description\":\"уже подарено достаточное количество\"}";
					}

				}
				else
				{
					// --- undo gift giving
					db.Query("DELETE FROM `gifts_to_give` WHERE `user_id`=\"" + user.GetID() + "\" AND `gift_id`=\"" + gift_id + "\";");
					ostResult << "{"
								"\"result\":\"success\","
								"\"status\":\"canceled\","
								"\"description\":\"резерв снят\","
								"\"recipient_user_id\":\"" + giftOwnerID + "\","
								"\"gifts_to_give\": [" << GetGiftToGiveListInJSONFormat("SELECT * FROM `gifts_to_give` WHERE `gift_id` in (SELECT `id` FROM `gifts` WHERE `user_id`=\"" + giftOwnerID + "\");", &db, &user) << "]"
								"}";
				}
			}
			else
			{
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": id should not be empty");
				}

				ostResult << "{\"result\":\"error\",\"description\":\"укажите название подарка\"}";
			}
		}

		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file json_response.htmlt was missing");
			throw CException("Template file json_response.htmlt was missing");
		}  // if(!indexPage.SetTemplate("AJAX_giveGiftVisible.htmlt"))

		MESSAGE_DEBUG("", action, "finish");
	}   // if(action == "AJAX_giveGiftVisible")


	if(action == "AJAX_getURLMetaData")
	{
		ostringstream   ostFinal;
		string		    loginFromUser;

		MESSAGE_DEBUG("", action, "start");

		if(user.GetLogin() == "Guest")
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": re-login required");
			}

			ostFinal << "{\"result\":\"error\",\"description\":\"re-login required\"}";
		}
		else
		{
			ostringstream	ost;
			string			url;

			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:" + action + ": start");
			}

			url = indexPage.GetVarsHandler()->Get("url");
			url = CleanUPText(url);

			if(url.empty() || url.length() > HTML_MAX_POLL_URL_LENGTH)
			{
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:" + action + ": url is empty OR too long OR imageTempSet is empty");

					ostFinal.str("");
					ostFinal << "{" << std::endl;
					ostFinal << "\"result\" : \"error\"," << std::endl;
					ostFinal << "\"description\" : \"ссылка пустая или слишком длинная\"" << std::endl;
					ostFinal << "}" << std::endl;
				}
			}
			else
			{
				{
					// --- !!! IMPORTANT !!! Create and destroy CHTML just once in the app
					CHTML		html("temp");

					if(html.PerformRequest(url))
					{
						unsigned long		feed_imageID = 0;
						string			  	feed_imageURL = "";
						string				feed_mediaType = "";

						if(html.isEmbedVideoHostedOnYoutube())
						{
							ostringstream   ost;

							{
								CLog log;
								log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:" + action + ": youtube video found");
							}

							feed_mediaType = "youtube_video";
							feed_imageURL = html.GetEmbedVideoURL();

							ost.str("");
							ost << "INSERT INTO `temp_media` SET "
								<< "`user_id`='" << user.GetID() << "', "
								<< "`mediaType`=\"" << feed_mediaType << "\",  "
								<< "`folder`='', "
								<< "`filename`=\"" << feed_imageURL << "\","
								<< "`eventTimestamp`=NOW();";

							 feed_imageID = db.InsertQuery(ost.str());
						}
						else if(html.GetPreviewImageFolder().length() && html.GetPreviewImagePrefix().length() && html.GetPreviewImageExtension().length())
						{
							// --- 1) check that image actually image
							// --- 2) move it to finalFolder
							// --- 3) submit to imageTempSet in DB
							struct ExifInfo exifInfo;
							string			finalFile, tmpFile2Check, tmpImageJPG;
							ostringstream	ost;

							{
								CLog log;
								log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:" + action + ": preview image found");
							}

							// --- init variables
							tmpFile2Check	= "/tmp/tmp_" + html.GetPreviewImagePrefix() + html.GetPreviewImageExtension();
							tmpImageJPG		= "/tmp/" + html.GetPreviewImagePrefix() + ".jpg";
							finalFile		= IMAGE_TEMP_DIRECTORY + "/" + html.GetPreviewImageFolder() + "/" + html.GetPreviewImagePrefix() + ".jpg";

							if(isFileExists(tmpFile2Check))
							{
								if(getFileSize(tmpFile2Check))
								{
#ifndef IMAGEMAGICK_DISABLE
									Magick::InitializeMagick(NULL);
#endif
									if(ImageSaveAsJpgToFeedFolder(tmpFile2Check, tmpImageJPG, exifInfo, &config))
									{

										{
											CLog	log;
											log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:" + action + ": chosen filename for feed image [" + finalFile + "]");
										}

										CopyFile(tmpImageJPG, finalFile);

										ost.str("");
										ost << "INSERT INTO `temp_media` SET "
											<< "`user_id`='" << user.GetID() << "',  "
											<< "`mediaType`='image',  "
											<< "`folder`='" << html.GetPreviewImageFolder() << "', "
											<< "`filename`='" << html.GetPreviewImagePrefix() << ".jpg', "
											<< "`eventTimestamp`=NOW();";

										feed_imageID = db.InsertQuery(ost.str());

										if(feed_imageID)
										{
											feed_imageURL = html.GetPreviewImageFolder() + "/" + html.GetPreviewImagePrefix() + ".jpg";
											feed_mediaType = "image";
										}
										else
										{
											MESSAGE_ERROR("", action, "fail to insert image info to temp_media table");
										}

										// --- Delete temporarily files
										unlink(tmpFile2Check.c_str());
										unlink(tmpImageJPG.c_str());
									}
									else
									{
										MESSAGE_ERROR("", action, "image [" + tmpFile2Check + "] is not valid image format (looks like attack)");
									}
								}
								else
								{
									MESSAGE_DEBUG("", action, "og:image is empty");
									unlink(tmpFile2Check.c_str());
								}
							}
							else
							{
								MESSAGE_DEBUG("", action, "og:image wasn't downloaded");
							}
						}
						else
						{
							CLog log;
							log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:" + action + ": there are no (youtube_video, preview image)");
						}
						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"success\",";
						ostFinal << "\"title\" : \"" << CleanUPText(html.GetTitle()) << "\",";
						ostFinal << "\"description\" : \"" << CleanUPText(html.GetDescription()) << "\",";
						ostFinal << "\"imageURL\" : \"" << feed_imageURL << "\",";
						ostFinal << "\"imageID\" : \"" << feed_imageID << "\",";

						// TODO: delete on Jan 1
/*
						if(feed_mediaType == "image")
						{
							ostFinal << "\"folder\" : \"" << html.GetPreviewImageFolder() << "\",";
							ostFinal << "\"filename\" : \"" << html.GetPreviewImagePrefix() << ".jpg,\",";
						}
						else if(feed_mediaType == "youtube_video")
						{
							ostFinal << "\"folder\" : \"\",";
							ostFinal << "\"filename\" : \"" << html.GetEmbedVideoURL() << "\",";
						}
*/
						ostFinal << "\"mediaType\" : \"" << feed_mediaType << "\"";
						ostFinal << "}";
					}
					else
					{
						{
							CLog	log;
							log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:" + action + ":ERROR: can't perform request" + string(url));
						}

						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"error\",";
						ostFinal << "\"description\" : \"Не удалось получить данные с сайта, проверьте корректность ссылки\"";
						ostFinal << "}";
					} // --- if curl.PerformRequest()
				} // --- scoping
			} // --- if url.length()
		} // --- if user == "Guest"

		indexPage.RegisterVariableForce("result", ostFinal.str());

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file json_response.htmlt was missing");
			throw CException("Template file json_response.htmlt was missing");
		}  // if(!indexPage.SetTemplate("json_response.htmlt"))

		MESSAGE_DEBUG("", action, "finish");
	}   // if(action == "AJAX_getURLMetaData")

	if(action == "AJAX_GotIt")
	{
		ostringstream   ostResult;
		string		    loginFromUser;

		MESSAGE_DEBUG("", action, "start");

		ostResult.str("");
		if(user.GetLogin() == "Guest")
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": re-login required");
			}

			ostResult << "{\"result\":\"error\",\"description\":\"re-login required\"}";
		}
		else
		{
			string	giftID = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));

			if(giftID.empty())
			{
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": id should not be empty");
				}

				ostResult << "{\"result\":\"error\",\"description\":\"ошибка передачи параметров\"}";
			}
			else
			{
				if(db.Query("SELECT * FROM `gifts` WHERE `id`=\"" + giftID + "\" AND `user_id`=\"" + user.GetID() + "\";"))
				{
					unsigned long	requested_quantity = stol(db.Get(0, "requested_quantity"));
					unsigned long	gained_quantity = stol(db.Get(0, "gained_quantity"));



					if(gained_quantity < requested_quantity)
					{
						++gained_quantity;
						db.Query("UPDATE `gifts` SET `gained_quantity`=\"" + to_string(gained_quantity) + "\" WHERE `id`=\"" + giftID + "\";");

						if(db.isError())
						{
							{
								CLog		log;
								log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: updating `gifts` table (" + db.GetErrorMessage() + ")");
							}

							ostResult << "{\"result\":\"error\",\"description\":\"ошибка БД\"}";
						}
						else
						{
							string		potentialGrantorUserID = "";

							if(db.Query("SELECT `user_id` FROM `gifts_to_give` WHERE `gift_id`=\"" + giftID + "\" AND `visibility`=\"visible\";") == 1)
							{
								potentialGrantorUserID = db.Get(0, "user_id");
							}
							else
							{
								{
									CLog		log;
									log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: no grantor OR number of grantors > 1");
								}
							}
							ostResult << "{\"result\":\"success\",\"gained_quantity\":\"" << gained_quantity << "\", \"potential_grantor_user\":[" << (potentialGrantorUserID.length() ? GetUserListInJSONFormat("SELECT * FROM `users` WHERE `id`=\"" + potentialGrantorUserID + "\";", &db, &user) : "") << "]}";
						}
					}
					else
					{
						ostResult << "{\"result\":\"error\",\"description\":\"Вам уже подарили, все что вы хотели\"}";
					}

				}
				else
				{
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ":ERROR: gift(" + giftID + ") not found or doesn't belongs to user(" + user.GetID() + ")");
					}
					ostResult << "{\"result\":\"error\",\"description\":\"подарок отсутствует в БД\"}";
				}
			}
		}

		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file json_response.htmlt was missing");
			throw CException("Template file json_response.htmlt was missing");
		}  // if(!indexPage.SetTemplate("AJAX_GotIt.htmlt"))

		MESSAGE_DEBUG("", action, "finish");
	}   // if(action == "AJAX_GotIt")

	if(action == "AJAX_GiftThank")
	{
		ostringstream   ostResult;
		string		    loginFromUser;

		MESSAGE_DEBUG("", action, "start");

		ostResult.str("");
		if(user.GetLogin() == "Guest")
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": re-login required");
			}

			ostResult << "{\"result\":\"error\",\"description\":\"re-login required\"}";
		}
		else
		{
			string	giftID = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("giftID"));
			string	grantorUserID = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("grantorUserID"));
			string	thankComment = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("thankComment"));

			if(giftID.empty() || grantorUserID.empty())
			{
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": giftID(" + giftID + ") and grantorUserID(" + grantorUserID + ") should not be empty");
				}

				ostResult << "{\"result\":\"error\",\"description\":\"ошибка передачи параметров\"}";
			}
			else
			{
				if(db.Query("SELECT * FROM `gifts` WHERE `id`=\"" + giftID + "\" AND `user_id`=\"" + user.GetID() + "\";"))
				{
					unsigned long	giftThankID = 0;

					giftThankID = db.InsertQuery("INSERT INTO `gift_thanks` SET `src_user_id`=\"" + user.GetID() + "\", `dst_user_id`=\"" + grantorUserID + "\", `gift_id`=\"" + giftID + "\", `comment`=\"" + thankComment + "\", `eventTimestamp`=UNIX_TIMESTAMP();");

					if(db.InsertQuery("INSERT INTO `users_notification` (`userId`, `actionTypeId`, `actionId`, `eventTimestamp`) VALUES (\"" + grantorUserID + "\", \"66\", \"" + to_string(giftThankID) + "\", UNIX_TIMESTAMP());"))
					{
						ostResult << "{\"result\":\"success\",\"description\":\"\"}";
					}
					else
					{
						{
							CLog	log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:" + action + ":ERROR: inserting into users_notification table");
						}
						ostResult << "{\"result\":\"error\",\"description\":\"ошибка БД\"}";
					}
				}
				else
				{
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ":ERROR: gift(" + giftID + ") not found or doesn't belongs to user(" + user.GetID() + ")");
					}
					ostResult << "{\"result\":\"error\",\"description\":\"подарок отсутствует в БД\"}";
				}
			}
		}

		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file json_response.htmlt was missing");
			throw CException("Template file json_response.htmlt was missing");
		}  // if(!indexPage.SetTemplate("AJAX_GiftThank.htmlt"))

		MESSAGE_DEBUG("", action, "finish");
	}   // if(action == "AJAX_GiftThank")giftID

	if(action == "AJAX_removeGiftEntry")
	{
		ostringstream   ostResult;
		string		    loginFromUser;

		MESSAGE_DEBUG("", action, "start");

		ostResult.str("");
		if(user.GetLogin() == "Guest")
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": re-login required");
			}

			ostResult << "{\"result\":\"error\",\"description\":\"re-login required\"}";
		}
		else
		{
			string	giftID = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));

			if(giftID.empty())
			{
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + "]:action == " + action + ": id should not be empty");
				}

				ostResult << "{\"result\":\"error\",\"description\":\"ошибка передачи параметров\"}";
			}
			else
			{
				if(db.Query("SELECT * FROM `gifts` WHERE `id`=\"" + giftID + "\" AND `user_id`=\"" + user.GetID() + "\";"))
				{
					auto	logo_folder		= db.Get(0, "logo_folder");
					auto	logo_filename	= db.Get(0, "logo_filename");

					if(logo_folder.length() && logo_filename.length())
					{
						auto  currLogo = config.GetFromFile("image_folders", "gift") + "/" + logo_folder + "/" + logo_filename;

						if(isFileExists(currLogo)) 
						{
							MESSAGE_DEBUG("", "", "remove current logo (" + currLogo + ")");

							unlink(currLogo.c_str());
						}
					}

					db.Query("DELETE FROM `gifts` WHERE `id`=\"" + giftID + "\";");

					if(db.isError())
					{
						MESSAGE_ERROR("", "", "removing form `gifts` table (" + db.GetErrorMessage() + ")");

						ostResult << "{\"result\":\"error\",\"description\":\"ошибка БД\"}";
					}
					else
					{
						ostResult << "{\"result\":\"success\",\"gifts\":[" + GetGiftListInJSONFormat("SELECT * FROM `gifts` WHERE `user_id`=\"" + user.GetID() + "\";", &db, &user) + "]}";
					}

					db.Query("DELETE FROM `users_notification` WHERE (`actionTypeId`=\"66\") AND (`actionId` in (SELECT `id` FROM `gift_thanks` WHERE `gift_id`=\"" + giftID + "\"))");
					if(db.isError())
					{
						MESSAGE_ERROR("", "", "removing form `users_notification` table (" + db.GetErrorMessage() + ")");
					}
					
					db.Query("DELETE FROM `gifts_to_give` WHERE `gift_id`=\"" + giftID + "\";");
					if(db.isError())
					{
						MESSAGE_ERROR("", "", "removing form `gifts_to_give` table (" + db.GetErrorMessage() + ")");
					}
				}
				else
				{
					MESSAGE_ERROR("", "", "gift(" + giftID + ") not found or doesn't belongs to user(" + user.GetID() + ")");

					ostResult << "{\"result\":\"error\",\"description\":\"подарок отсутствует в БД\"}";
				}
			}
		}

		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file json_response.htmlt was missing");
			throw CException("Template file json_response.htmlt was missing");
		}  // if(!indexPage.SetTemplate("AJAX_removeGiftEntry.htmlt"))

		MESSAGE_DEBUG("", action, "finish");
	}   // if(action == "AJAX_removeGiftEntry")


	MESSAGE_DEBUG("", action, "finish");

	indexPage.OutTemplate();

	}
/*
	catch(CExceptionRedirect &c) {
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << string(__func__) + ":: catch CRedirectHTML: exception used for redirection";
		log.Write(DEBUG, ost.str());

		c.SetDB(&db);

		if(!indexPage.SetTemplate(c.GetTemplate())) {

			ost.str("");
			ost << string(__func__) + ":: catch CRedirectHTML: ERROR, template redirect.htmlt not found";
			log.Write(ERROR, ost.str());

			throw CException("Template file was missing");
		}

		indexPage.RegisterVariableForce("content", "redirect page");
		indexPage.OutTemplate();

	}
*/
	catch(CExceptionHTML &c)
	{
		CLog	log;

		c.SetLanguage(indexPage.GetLanguage());
		c.SetDB(&db);

		log.Write(DEBUG, string(__func__) + ": catch CExceptionHTML: DEBUG exception reason: [", c.GetReason(), "]");

		if(!indexPage.SetTemplate(c.GetTemplate()))
		{
			return(-1);
		}
		indexPage.RegisterVariable("content", c.GetReason());
		indexPage.OutTemplate();
		return(0);
	}
	catch(CException &c)
	{
		CLog 	log;

		if(!indexPage.SetTemplateFile("templates/error.htmlt"))
		{
			return(-1);
		}

		log.Write(ERROR, string(__func__) + ": catch CException: exception: ERROR  ", c.GetReason());

		indexPage.RegisterVariable("content", c.GetReason());
		indexPage.OutTemplate();
		return(-1);
	}
	catch(exception& e)
	{
		CLog 	log;
		log.Write(ERROR, string(__func__) + ": catch(exception& e): catch standard exception: ERROR  ", e.what());

		if(!indexPage.SetTemplateFile("templates/error.htmlt"))
		{
			return(-1);
		}
		indexPage.RegisterVariable("content", e.what());
		indexPage.OutTemplate();
		return(-1);
	}

	return(0);
}

