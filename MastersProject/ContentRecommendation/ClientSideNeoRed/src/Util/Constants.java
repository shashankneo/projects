package Util;

public class Constants {
	public static int MAX_LENGTH_FN = 25;
	public static int DOWNLOAD_PER = 60;
	public static int TITLE_MAX = 60;
	public static int CONTENT_MAX = 80;
	public static String user = "username";
	public static String keycode = "keycode";
	public static String laptop = "laptop";
	public static String chrome_history = "chrome_history";
	public static String logDir = "Log";
	public static String logFile = "logFile.txt";
	public static String showCenter = "showCenter";
	public static String showBottom = "showBottom";
	
	public static String dir_links = "linksDir";
	public static String file_chrome = "chrome.json";
	public static String file_ultimate = "ultimate.json";
	public static String dir_twitter = "twitterDir";
	public static String dir_facebook = "facebookDir";
	public static String dir_data = "dataDir";
	public static String dir_final = "Final_dir";
	
	public static String pref_dateChrome ="dateChrome";
	//download everything transferred from mobile
	public static String pref_dateDownloadUltimate ="pref_dateDownloadUltimate";
	//download everything transferred from laptop
	public static String pref_dateDownloadLaptop ="dateDownloadLaptop";
	public static String pref_username ="pref_username";
	public static String pref_keycode ="pref_keycode";
	//Pref for last date for getting mobile ultimate data from server
	public static String pref_dateUltimateLinks = "dateUltimateLinks";
	//Pref for last date for sharing laptop data
	public static String pref_dateShareLaptopData = "dateShareLaptopData";
	//Pref for last date for sharing recommendations
	public static String pref_dateShareRecommendations = "dateShareRecommendations";
	//Pref for last date for sharing recommendations
	public static String pref_dateShareFeedback = "dateShareFeedback";
	
	public static String cmd_laptopRegistration = "cmd_laptopRegistration";
	public static String cmd_laptopRegistrationDone = "cmd_laptopRegistrationDone";
	public static String cmd_RecvlaptopData = "cmd_RecvlaptopData";
	public static String cmd_RecvlaptopDataDone = "cmd_RecvlaptopDataDone";
	public static String cmd_SendlaptopData = "cmd_SendlaptopData";
    public static String cmd_SendlaptopDataDone = "cmd_SendlaptopDataDone";
    public static String cmd_SendlaptopRecommendation = "cmd_SendlaptopRecommendation";
    public static String cmd_SendlaptopRecommendationDone = "cmd_SendlaptopRecommendationDone";
    public static String cmd_SendlaptopFeedback = "cmd_SendlaptopFeedback";
    public static String cmd_SendlaptopFeedbackDone = "cmd_SendlaptopFeedbackDone";
    
	public static final String SERVER_IP = "130.245.145.207";
    //"130.245.41.8";
	//130.245.145.207
    public static int   connection_duration = 120000;
    public static int port1 = 54556;
    public static int port2 = 54778;
    public static int READ_BUFFER = 1024*200;
    public static int WRITE_BUFFER = 1024*200;

	
}
