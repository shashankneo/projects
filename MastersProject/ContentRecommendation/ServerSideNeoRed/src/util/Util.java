package util;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;


public class Util {
	static public boolean writeUltimateJson(String foldername, String jsonString) {


        boolean result = false;
        if(jsonString == null)
        	return false;
        File f = new File(foldername, Constants.ULTIMATE_JSON);
        FileWriter file = null;
        try {
            f.createNewFile();
            file = new FileWriter(f);
            file.write(jsonString);
            System.out.println("Successfully written todays ultimate json file="+f.toString());
            result = true;
        } catch (IOException e) {
            System.out.println("Exception in writing todays fb json file");

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
	static public boolean writeChromeJson(String foldername, String jsonString) {


        boolean result = false;
        if(jsonString == null)
        	return false;
        File f = new File(foldername, Constants.file_chrome);
        FileWriter file = null;
        try {
            f.createNewFile();
            file = new FileWriter(f);
            file.write(jsonString);
            System.out.println("Successfully written todays ultimate json file="+f.toString());
            result = true;
        } catch (IOException e) {
            System.out.println("Exception in writing todays fb json file");

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

}
