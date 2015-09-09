package util;

import java.util.UUID;


public class Constants {
	
	public static String server = "server";
	public static String laptop = "laptop";
	public static String mobile = "mobile";
	public static String errors = "errors";
	public static int port1 = 54556;
	public static int port2 = 54778;
	public static int READ_BUFFER = 1024*300;
    public static int WRITE_BUFFER = 1024*300;
	public static String DATA_DIR ="dataDir";
	public static String USER_DIR ="userDir";
	public static String LINKS_DIR ="linksDir";
	public static String RECOMMENDATION_DIR ="recommendationDir";
	public static String FEEDBACK_DIR ="feedbackDir";
	public static String ULTIMATE_JSON = "ultimate.json";
	public static String file_chrome = "chrome.json";
	public static String logDir = "Log";
	public static String logFile = "logFile.txt";
	public static String randomString(){
		String uuid = UUID.randomUUID().toString().replaceAll("-", "");
		return uuid;
	}    
    
}
