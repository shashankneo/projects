package Util;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.URLConnection;
import java.nio.file.CopyOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.prefs.Preferences;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Util {
	static Logger logger = LoggerFactory.getLogger(Util.class);
	public static String getChromeFolder(){
		String workingDirectory;
		//here, we assign the name of the OS, according to Java, to a variable...
		 String OS = (System.getProperty("os.name")).toUpperCase();
		//to determine what the workingDirectory is.
		//if it is some version of Windows
		if (OS.contains("WIN"))
		{
		    //it is simply the location of the "AppData" folder
		    workingDirectory = System.getenv("AppData");
		    workingDirectory = workingDirectory+"\\Google\\Chrome\\User Data\\Default\\";
		}
		//Otherwise, we assume Linux or Mac
		else
		{
		    //in either case, we would start in the user's home directory
		    workingDirectory = System.getProperty("user.home");
		    //if we are on a Mac, we are not done, we look for "Application Support"
		    workingDirectory += "/Library/Application Support";
		    workingDirectory = workingDirectory + "/Google/Chrome/Default/History";
		}
		
		
		return workingDirectory;
	}
	
	public static boolean copyFiles(String source, String desti){
	boolean result = false;
	
	Path FROM = Paths.get(source);
	Path TO = Paths.get(desti);
	CopyOption[] options = new CopyOption[]{
	  StandardCopyOption.REPLACE_EXISTING,
	  StandardCopyOption.COPY_ATTRIBUTES
	}; 
	try {
		java.nio.file.Files.copy(FROM, TO, options);
		result = true;
	} catch (IOException e) {
		// TODO Auto-generated catch block
		e.printStackTrace();
	}
	
	return result;	
	}
	
	public static void setPreferences(String key, String value){
		Preferences prefs = Preferences.userRoot().node("utils");
		prefs.put(key, value);
	}
	
	public static String getPreferences(String key){
		Preferences prefs = Preferences.userRoot().node("utils");
		String val = prefs.get(key, null);
		return val;
	}
	public static void createLogFile(){
		File f = new File(Constants.logDir);
		if(f.exists()==false){
			f.mkdirs();
		}
		File logFile = new File(Constants.logDir,Constants.logFile);
		if(logFile.exists()==false){
			try {
				logFile.createNewFile();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
	
	static public boolean writeToFile(String jsonString, String dir, String file_name) {


        boolean result = false;
        File folder = new File(dir);
        if(folder.exists()==false){
        	folder.mkdirs();
        }
        File f = new File(dir,file_name);
        FileWriter file = null;
        try {
            f.createNewFile();
            file = new FileWriter(f);
            file.write(jsonString);
            
            result = true;
        } catch (IOException e) {
            logger.error("Exception in writing file for="+dir+"/"+file_name);

        } finally {
            try {
                file.flush();
                file.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
        return result;
    }
	
	 public static String readFile(String filename) {
	        String result = null;
	        try {
	            BufferedReader br = new BufferedReader(new FileReader(filename));
	            StringBuilder sb = new StringBuilder();
	            String line = br.readLine();
	            while (line != null) {
	                sb.append(line);
	                line = br.readLine();
	            }
	            result = sb.toString();
	        } catch(Exception e) {
	            e.printStackTrace();
	        }
	        return result;
	    }
	 
	 
	 static public String getDateString(Date today ){
	        SimpleDateFormat sdf = new SimpleDateFormat("dd-MM-yyyy");
	        String date = sdf.format(today);
	        return date;
	    }
	 
	 static public boolean writeHtmlFile(String link, String dir){
		 URL url;
		 boolean result = false;
		 File folder = new File(dir);
	        if(folder.exists()==false){
	        	folder.mkdirs();
	        }
			try {
				// get URL content
				url = new URL(link);
				URLConnection conn = url.openConnection();
	 
				// open the stream and put it into BufferedReader
				BufferedReader br = new BufferedReader(
	                               new InputStreamReader(conn.getInputStream()));
	 
				String inputLine;
	 
				//save to this filename
				String fileName = url.toString();
				if(fileName.length()>Constants.MAX_LENGTH_FN)
					fileName = fileName.substring(0, Constants.MAX_LENGTH_FN);
				fileName = fileName.replaceAll("[^A-Za-z0-9]", "");
				File file = new File(dir,fileName);
	 
				if (!file.exists()) {
					file.createNewFile();
				}
	 
				//use FileWriter to write file
				FileWriter fw = new FileWriter(file.getAbsoluteFile());
				BufferedWriter bw = new BufferedWriter(fw);
	 
				while ((inputLine = br.readLine()) != null) {
					bw.write(inputLine);
				}
	 
				bw.close();
				br.close();
	 
				result = true;
	 
			} catch (Exception e) {
				logger.error("Exception while downloading ="+e.toString());
			} 
			return result;
	 }
	 public static void setPrefToNull(){
		 Preferences prefs = Preferences.userRoot().node("utils");
		 prefs.put(Constants.pref_dateChrome, "");
		 prefs.put(Constants.pref_dateDownloadUltimate, "");
		 prefs.put(Constants.pref_dateDownloadLaptop, "");
		 prefs.put(Constants.pref_username, "");
		 prefs.put(Constants.pref_keycode, "");
		 prefs.put(Constants.pref_dateUltimateLinks, "");
		 prefs.put(Constants.pref_dateShareLaptopData, "");
		 prefs.put(Constants.pref_dateShareRecommendations, "");
		 prefs.put(Constants.pref_dateShareFeedback, "");
	 }
}
