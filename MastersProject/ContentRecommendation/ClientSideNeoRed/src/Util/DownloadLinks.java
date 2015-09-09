package Util;

import java.io.File;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.quartz.Job;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class DownloadLinks implements Job{
	   private Logger logger = LoggerFactory.getLogger(DownloadLinks.class);
	   private String ultimateDownloadDate = null;
	   private String linksDownloadDate = null;
	   public void execute(JobExecutionContext context)
	            throws JobExecutionException {
	                logger.info("Lets download all links on laptop begun"); 
	               // timeToDownloadMobile();
	              //  timeToDownloadLaptop();
	            }
	   public void timeToDownloadMobile(){
		   boolean result = false;
			String lastDownloadedDate = Util.getPreferences(Constants.pref_dateDownloadUltimate);
			String todayDate = Util.getDateString(new Date());
			Calendar downloadDate = null;
			
			while(!todayDate.equals(lastDownloadedDate)){
				//Find out next day to download
				if(lastDownloadedDate == null){
					downloadDate = new GregorianCalendar();
			    }
			    else{
			    	SimpleDateFormat format = new SimpleDateFormat("dd-MM-yyyy");
			    	try {
						Date temp =format.parse(lastDownloadedDate );
						downloadDate =GregorianCalendar.getInstance();
						downloadDate.setTime(temp);
						
						//Above sets the date to last downloaded date. Now move ahead by a day.
						downloadDate.add(Calendar.DAY_OF_MONTH, 1);
					} catch (ParseException e) {
						// TODO Auto-generated catch block
						downloadDate = null;
						
					}
			    }
				result = getUltimate(downloadDate);
				if(result == false){
					return;
				}
				lastDownloadedDate = this.ultimateDownloadDate;
				Util.setPreferences(Constants.pref_dateDownloadUltimate, lastDownloadedDate);
			}
	   }
	   
	   boolean getUltimate(Calendar date){
		   boolean result = false;
		   ultimateDownloadDate = Util.getDateString(date.getTime());
		   String jsonData = Util.readFile( System.getProperty("user.dir")+File.separator+Constants.dir_links+
	        		File.separator+ultimateDownloadDate+File.separator+ Constants.file_ultimate);
		   if(jsonData == null){
			   return result;
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
		   boolean result = false;
			String lastDownloadedDate = Util.getPreferences(Constants.pref_dateDownloadLaptop);
			String todayDate = Util.getDateString(new Date());
			Calendar downloadDate = null;
			
			while(!todayDate.equals(lastDownloadedDate)){
				//Find out next day to download
				if(lastDownloadedDate == null){
					downloadDate = new GregorianCalendar();
			    }
			    else{
			    	SimpleDateFormat format = new SimpleDateFormat("dd-MM-yyyy");
			    	try {
						Date temp =format.parse(lastDownloadedDate );
						downloadDate =GregorianCalendar.getInstance();
						downloadDate.setTime(temp);
						
						//Above sets the date to last downloaded date. Now move ahead by a day.
						downloadDate.add(Calendar.DAY_OF_MONTH, 1);
					} catch (ParseException e) {
						// TODO Auto-generated catch block
						downloadDate = null;
						
					}
			    }
				result = getLaptopDownload(downloadDate);
				if(result == false){
					return;
				}
				lastDownloadedDate = this.linksDownloadDate;
				Util.setPreferences(Constants.pref_dateDownloadLaptop, lastDownloadedDate);
			}
	   }
	   
	   boolean getLaptopDownload(Calendar date){
		   boolean result = false;
		   linksDownloadDate = Util.getDateString(date.getTime());
		   String jsonData = Util.readFile( System.getProperty("user.dir")+File.separator+Constants.dir_links+
	        		File.separator+linksDownloadDate+File.separator+ Constants.file_chrome);
		   if(jsonData == null){
			   return result;
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