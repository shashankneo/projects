package network;

import java.io.File;
import java.io.FileFilter;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import Util.Constants;
import Util.Util;

public class DownloadAllData {
	private Logger logger = LoggerFactory.getLogger(DownloadAllData.class);

    public void downloadEverything(){
    	timeToDownloadMobile();
    	timeToDownloadLaptop();
    }
	public void timeToDownloadMobile(){
		   boolean result = true;
		   String lastDownloadedDate = getNextUltimateDownloadDate();
			
		
			while(result){
				if(lastDownloadedDate!=null){
					result = getUltimate(lastDownloadedDate);
				}
				else{
					result = true;
					break;
				}
				if(result)
					Util.setPreferences(Constants.pref_dateDownloadUltimate, lastDownloadedDate);
				lastDownloadedDate = getNextUltimateDownloadDate();
			}
				
	   }
	   private String getNextUltimateDownloadDate(){
		   String lastFolder = Util.getPreferences(Constants.pref_dateDownloadUltimate);
	        File f = new File(System.getProperty("user.dir") + File.separator + Constants.dir_links);
	        File[] files = null;
	        //if data folder exists
	        if (f.exists()!=true) {
	            lastFolder = null;
	        }
	        else{
	            files = f.listFiles(new FileFilter() {
	        	    public boolean accept(File file) {
	        	        return !file.isHidden();
	        	    }
	        	});
	            if(files.length == 0)
	            	return null;
	            Arrays.sort(files);
	            //If lastFolder returned is today one then simply return null
	            if(lastFolder !=null && lastFolder.equals(Util.getDateString(new Date()))==true){
	                return null;
	            }
	            else{
	            	if(lastFolder == null)
	            		return files[0].getName();
	                String yesterDay = Util.getDateString(new Date(System.currentTimeMillis() -
	                        1000L * 60L * 60L * 24L));
	                //Yesterday folder was last sent
	                if(lastFolder != null && lastFolder.equals(yesterDay)){
	                    return Util.getDateString(new Date());
	                }
	                else{
	                    int count = 0;
	                    for(File obj : files){
	                    	count = count +1;
	                        if(obj.getName().equals(lastFolder)){  
	                            break;
	                        }
	                    }
	                    if(count != files.length){
	                        lastFolder = files[count].getName();
	                    }
	                    else{
	                        lastFolder = null;
	                    }
	                }
	            }
	        }
	        return lastFolder;
	   }
	   boolean getUltimate(String ultimateDownloadDate){
		   boolean result = false;
		   String jsonData = Util.readFile( System.getProperty("user.dir")+File.separator+Constants.dir_links+
	        		File.separator+ultimateDownloadDate+File.separator+ Constants.file_ultimate);
		   //Skip if data cant be read
		   if(jsonData == null){
			   return true;
		   }
		   JSONParser jsonParser=new JSONParser();
		  try {
			  JSONObject jsonObj = (JSONObject)jsonParser.parse(jsonData);
			  int i;
			  if(jsonObj.containsKey("tweets")) {
					JSONArray jsonTweets = (JSONArray) jsonObj.get("tweets");
					int length = jsonTweets.size();
					for( i =0;i<length; i++){
						JSONObject tweetObj = (JSONObject) jsonTweets.get(i);
						String msg = (String) tweetObj.get("tweet");
						String file_name = msg;
						if(msg.length()>Constants.MAX_LENGTH_FN)
							file_name =	msg.substring(0, Constants.MAX_LENGTH_FN);
							file_name = file_name.replaceAll("[^A-Za-z0-9]", "");
						result = Util.writeToFile(msg, System.getProperty("user.dir")+File.separator+Constants.dir_data+
	        		File.separator+ultimateDownloadDate+File.separator+Constants.dir_twitter, file_name);
						if(result == false)
							return result;
					}
				}
			 if(jsonObj.containsKey("fb_msg")){
					JSONArray jsonFbs = (JSONArray) jsonObj.get("fb_msg");
					int length = jsonFbs.size();
					for(i =0;i<length; i++){
						JSONObject fbObj = (JSONObject) jsonFbs.get(i);
						String msg = (String) fbObj.get("message");
						String file_name = msg;
						if(msg.length()>Constants.MAX_LENGTH_FN)
							file_name =	msg.substring(0, Constants.MAX_LENGTH_FN);
						file_name = file_name.replaceAll("[^A-Za-z0-9]", "");
						result = Util.writeToFile(msg, System.getProperty("user.dir")+File.separator+Constants.dir_data+
	        		File.separator+ultimateDownloadDate+File.separator+Constants.dir_facebook, file_name);
						if(result == false)
							return result;
					}
				}
			if(jsonObj.containsKey("links")){
					JSONArray jsonLinks = (JSONArray) jsonObj.get("links");
					int length = jsonLinks.size();
					int count_links = 0;
					for(i =0;i<length; i++){
						JSONObject linkObj = (JSONObject) jsonLinks.get(i);
						String link = (String) linkObj.get("link");
						result = Util.writeHtmlFile(link, System.getProperty("user.dir")+File.separator+Constants.dir_data+
				        		File.separator+ultimateDownloadDate );
						if(result == true)
							count_links++;
					}
					if(((count_links*100)/length)<Constants.DOWNLOAD_PER){
						return false;
					}
				}
			
		} catch (Exception e) {
			// TODO Auto-generated catch block
			
		}
		  if(result == true){
				logger.info("Successfully performed ultimate download  for="+ultimateDownloadDate);
			}
			else{
				logger.error("UnSuccessfully performed ultimate download  for="+ultimateDownloadDate);
			}
		   return result;
	   }
	   
	   public void timeToDownloadLaptop(){
		   boolean result = true;
		   String lastDownloadedDate = getNextLaptopDownloadDate();
		   while(result){
				if(lastDownloadedDate!=null){
					result = getLaptopDownload(lastDownloadedDate);
				}
				else{
					result = true;
					break;
				}
				if(result)
					Util.setPreferences(Constants.pref_dateDownloadLaptop, lastDownloadedDate);
				lastDownloadedDate = getNextLaptopDownloadDate();
			}
				
	   }
	   private String getNextLaptopDownloadDate(){
		   String lastFolder = Util.getPreferences(Constants.pref_dateDownloadLaptop);
	        File f = new File(System.getProperty("user.dir") + File.separator + Constants.dir_links);
	        File[] files = null;
	        //if data folder exists
	        if (f.exists()!=true) {
	            lastFolder = null;
	        }
	        else{
	            files = f.listFiles(new FileFilter() {
	        	    public boolean accept(File file) {
	        	        return !file.isHidden();
	        	    }
	        	});
	            if(files.length == 0)
	            	return null;
	            Arrays.sort(files);
	            //If lastFolder returned is today one then simply return null
	            if(lastFolder !=null && lastFolder.equals(Util.getDateString(new Date()))==true){
	                return null;
	            }
	            else{
	            	if(lastFolder == null)
	            		return files[0].getName();
	                String yesterDay = Util.getDateString(new Date(System.currentTimeMillis() -
	                        1000L * 60L * 60L * 24L));
	                //Yesterday folder was last sent
	                if(lastFolder != null && lastFolder.equals(yesterDay)){
	                    return Util.getDateString(new Date());
	                }
	                else{
	                    int count = 0;
	                    for(File obj : files){
	                    	count = count +1;
	                        if(obj.getName().equals(lastFolder)){  
	                            break;
	                        }
	                    }
	                    if(count != files.length){
	                        lastFolder = files[count].getName();
	                    }
	                    else{
	                        lastFolder = null;
	                    }
	                }
	            }
	        }
	        return lastFolder;
	   }
	   boolean getLaptopDownload(String linksDownloadDate){
		   boolean result = false;
		   String jsonData = Util.readFile( System.getProperty("user.dir")+File.separator+Constants.dir_links+
	        		File.separator+linksDownloadDate+File.separator+ Constants.file_chrome);
		   if(jsonData == null){
			   return true;
		   }
		   JSONParser jsonParser=new JSONParser();
		   JSONArray writeLinks = new JSONArray();
		  try {
			  JSONArray jsonLinks = (JSONArray)jsonParser.parse(jsonData);
			  int i;
			  int length = jsonLinks.size();
			  int count_links = 0;
			  for(i =0;i<length; i++){
			  JSONObject linkObj = (JSONObject) jsonLinks.get(i);
						String link = (String) linkObj.get("link");
						String download = (String) linkObj.get("download");
						if(download.equals("no")){
						result = Util.writeHtmlFile(link, System.getProperty("user.dir")+File.separator+Constants.dir_data+
				        		File.separator+linksDownloadDate );
						}
						else{
							result = false;
						}
						
						if(result == true){
							count_links++;
							linkObj.put("download", "yes");
						}
						writeLinks.add(linkObj);
					}
					if(((count_links*100)/length)<Constants.DOWNLOAD_PER){
						{
							result = false;
							
						}
					}
				
			
		} catch (Exception e) {
			// TODO Auto-generated catch block
			
		}
		  if(result == true){
				logger.info("Successfully performed all laptop download  for="+linksDownloadDate);
			}
			else{
				logger.error("UnSuccessfully performed all laptop download  for="+linksDownloadDate);
			}
		  Util.writeToFile(writeLinks.toJSONString(), System.getProperty("user.dir")+File.separator+Constants.dir_links+
	        		File.separator+linksDownloadDate, Constants.file_chrome);
		   return result;
	   }
}
