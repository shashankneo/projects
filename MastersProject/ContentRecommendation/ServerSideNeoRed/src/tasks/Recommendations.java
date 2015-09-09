package tasks;

import java.io.File;
import java.io.FileFilter;
import java.util.Arrays;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import network.SomeResponse;
import util.Constants;
import util.Errors;
import util.LaptopConstants;
import util.MobileConstants;
import util.Util;
import db.Sqllitedb;

public class Recommendations {
	private Logger logger = LoggerFactory.getLogger(Recommendations.class);
	public SomeResponse doRecvMobileRecomm(String username, String keycode,String dateFolder){
		
		SomeResponse response = new SomeResponse();
		String nextFolder = null;
		boolean result = false;
		try {
			Sqllitedb dbInstance = Sqllitedb.getInstance();
			result = dbInstance.checkCredentials(username,keycode);
			if(result == true){
			   String folder_path = System.getProperty("user.dir")+File.separator+Constants.DATA_DIR+File.separator
					   +username+File.separator+Constants.RECOMMENDATION_DIR;
			   File folder = new File(folder_path);
		        if(folder.exists()==true){
		        	File[] files = folder.listFiles(new FileFilter() {
		        	    public boolean accept(File file) {
		        	        return !file.isHidden();
		        	    }
		        	});
		        	Arrays.sort(files);
		        	int count = 0;
		        	if(files.length == 0){
		        		//no file to send
		        		nextFolder = null;
		        	}
		        	else if(dateFolder == null){
		        		nextFolder = files[count].getName();
		        	}
		        	else{
		        		String topFolder = null;
			            topFolder = files[files.length - 1].getName() ;
			            if(topFolder != null && dateFolder.equals(topFolder)){
			            	nextFolder = null;
			            }
			            else{
			            	for(File obj : files){
		        			 	count = count +1;
		                        if(obj.getName().equals(dateFolder)){ 
		                            break;
		                        }
		                       
		                    }
		                    if(count < files.length){
		                    	nextFolder = files[count].getName();
		                    }
		                    else{
		                    	nextFolder = null;
		                    }
			            }
			        }
                   
		        	response.date = nextFolder;
		        	String jsonData = null;
		        	if(nextFolder!=null){
		        		jsonData = Util.readFile(folder_path+File.separator+nextFolder+File.separator+
		        				Constants.ULTIMATE_JSON);
		        	}
		        	response.data = jsonData;
		        }
		      
			}
				
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			result = false;
		}
		if(result == true){
			
			response.command = MobileConstants.cmd_RecvmobileRecommDone;
			logger.info("Recommendation successfully received by mobile = " + nextFolder);
		}
		else{
			response = Errors.generateError(Errors.TRY_AGAIN_LATER);
			logger.error("Recommendation unsuccessfully received by mobile = " + nextFolder);
		}
		return response;
	}
	
	public SomeResponse doSendMobileFeedback(String username, String keycode, String data, String dateFolder){
		SomeResponse response;
		
		boolean result = false;
		try {
			Sqllitedb dbInstance = Sqllitedb.getInstance();
			result = dbInstance.checkCredentials(username,keycode);
			if(result == true){
			   String folder_path = System.getProperty("user.dir")+ File.separator + Constants.DATA_DIR+
					   File.separator + username+File.separator+Constants.FEEDBACK_DIR + 
				File.separator + Constants.mobile + File.separator + dateFolder;
			   File folder = new File(folder_path);
		        if(folder.exists()==false){
		        	folder.mkdirs();
		        }
		        if(data == null)
		        	result = true;
		        else
		        	result = Util.writeUltimateJson(folder_path, data);
			}
				
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			result = false;
		}
		if(result == true){
			response = new SomeResponse();
			response.command = MobileConstants.cmd_SendmobileFeedbackDone;
			response.data = keycode;
			logger.info("Feedback successfully sent by mobile = " + dateFolder);
		}
		else{
			response = Errors.generateError(Errors.TRY_AGAIN_LATER);
			logger.error("Feedback unsuccessfully sent by mobile = " + dateFolder);
		}
		return response;
	}
	
	
	public SomeResponse doSendLaptopRecommendation(String username, String keycode, String data, String dateFolder){
		SomeResponse response;
		
		boolean result = false;
		try {
			Sqllitedb dbInstance = Sqllitedb.getInstance();
			result = dbInstance.checkCredentials(username,keycode);
			if(result == true){
			   String folder_path = System.getProperty("user.dir")+ File.separator + Constants.DATA_DIR+
					   File.separator + username+File.separator+Constants.RECOMMENDATION_DIR + 
				File.separator + dateFolder;
			   File folder = new File(folder_path);
		        if(folder.exists()==false){
		        	folder.mkdirs();
		        }
		        if(data == null)
		        	result = true;
		        else
		        	result = Util.writeUltimateJson(folder_path, data);
			}
				
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			result = false;
		}
		if(result == true){
			response = new SomeResponse();
			response.command = LaptopConstants.cmd_SendlaptopRecommendationDone;
			response.data = keycode;
			logger.info("Recommendation successfully sent by laptop = " + dateFolder);
		}
		else{
			response = Errors.generateError(Errors.TRY_AGAIN_LATER);
			logger.error("Recommendation unsuccessfully sent by laptop = " + dateFolder);
		}
		return response;
	}
	
	public SomeResponse doSendLaptopFeedback(String username, String keycode, String data, String dateFolder){
		SomeResponse response;
		
		boolean result = false;
		try {
			Sqllitedb dbInstance = Sqllitedb.getInstance();
			result = dbInstance.checkCredentials(username,keycode);
			if(result == true){
			   String folder_path = System.getProperty("user.dir")+ File.separator + Constants.DATA_DIR+
					   File.separator + username+File.separator+Constants.FEEDBACK_DIR + 
				File.separator + Constants.laptop + File.separator + dateFolder;
			   File folder = new File(folder_path);
		        if(folder.exists()==false){
		        	folder.mkdirs();
		        }
		        if(data == null)
		        	result = true;
		        else
		        	result = Util.writeUltimateJson(folder_path, data);
			}
				
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			result = false;
		}
		if(result == true){
			response = new SomeResponse();
			response.command = LaptopConstants.cmd_SendlaptopFeedbackDone;
			response.data = keycode;
			logger.info("Feedback successfully sent by laptop = " + dateFolder);
		}
		else{
			response = Errors.generateError(Errors.TRY_AGAIN_LATER);
			logger.error("Feedback unsuccessfully sent by laptop = " + dateFolder);
		}
		return response;
	}
}
