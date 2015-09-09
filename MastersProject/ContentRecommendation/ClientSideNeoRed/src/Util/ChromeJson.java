package Util;



import java.io.File;
import java.nio.file.Files;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.Statement;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.quartz.Job;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;



public class ChromeJson{
	   private Logger logger = LoggerFactory.getLogger(ChromeJson.class);
	   private String downloadDate = null;
	   
	public void createChromeJson(){
		logger.info("Chrome links download on laptop begun"); 
		boolean result = false;
		String lastDownloadedDate = Util.getPreferences(Constants.pref_dateChrome);
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
			result = getJson(downloadDate);
			//if(result == false){
			//	return;
		  //	}
			lastDownloadedDate = this.downloadDate;
			Util.setPreferences(Constants.pref_dateChrome, lastDownloadedDate);
		}
		
		
		
	}
	
	private boolean getJson(Calendar date){
		boolean result = false;
		String jsonResult = null;
		String srcFolder = Util.getChromeFolder();
		String dstFolder = System.getProperty("user.dir")+File.separator+Constants.chrome_history;
		boolean copy = Util.copyFiles(srcFolder,dstFolder);
		downloadDate = Util.getDateString(date.getTime());
		if(copy==false)
			return false;
		Connection connection = null;
	    ResultSet resultSet = null;
	    PreparedStatement statement = null;

	    
	    
        // reset hour, minutes, seconds and millis
        date.set(Calendar.HOUR_OF_DAY, 0);
        date.set(Calendar.MINUTE, 0);
        date.set(Calendar.SECOND, 0);
        date.set(Calendar.MILLISECOND, 0);
        Date start_date = new Date(date.getTimeInMillis());
        // next day
        date.add(Calendar.DAY_OF_MONTH, 1);
        Date end_date = new Date(date.getTimeInMillis());
        JSONArray chromelinkList = new JSONArray();
        int count = 0;
	    try 
	        {
	        Class.forName ("org.sqlite.JDBC");
	        connection = DriverManager
	            .getConnection ("jdbc:sqlite:"+dstFolder);
	        connection.setAutoCommit(false);
	    

	        java.sql.Date sD = new java.sql.Date(start_date.getTime());
       	 java.sql.Date eD = new java.sql.Date(end_date.getTime());
	        statement = connection.prepareStatement(
		              "SELECT * From urls WHERE datetime(last_visit_time/1000000-11644473600,'unixepoch','localtime') BETWEEN '"+sD +"' AND '"+eD+"'");

		    resultSet= statement.executeQuery();      
	        while (resultSet.next ()) 
	            {
	        		JSONObject urlObj = new JSONObject();
	        		urlObj.put("link", resultSet.getString ("url"));
	        		urlObj.put("download", "no");
	        		count = count + 1;
	        		chromelinkList.add(urlObj);
	        		
	            }
	        
	        result = Util.writeToFile(chromelinkList.toJSONString(), System.getProperty("user.dir")+File.separator+Constants.dir_links+
	        		File.separator+downloadDate, Constants.file_chrome);
	        
	        } 

	    catch (Exception e) 
	        {
	        e.printStackTrace ();
	        } 

	    finally 
	        {
	        try 
	            {
	            resultSet.close ();
	            statement.close ();
	            connection.close ();
	            } 

	        catch (Exception e) 
	            {
	            e.printStackTrace ();
	            }
	        }
	    try {
	    	File file = new File(dstFolder);
	    	boolean del_file = file.delete();
	    } catch (Exception x) {
	        
	    } 
		if(result == true){
			logger.info("Successfully written chrome links = "+count+" extracted "
					+ "from laptop for="+downloadDate);
		}
		else{
			logger.info("Something failed while writing chrome links = "+count+" extracted "
					+ "from laptop for="+downloadDate);
		}
		
		
		return result;
	}
}

