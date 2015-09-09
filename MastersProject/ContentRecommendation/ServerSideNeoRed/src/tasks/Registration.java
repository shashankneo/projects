package tasks;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import network.SomeResponse;
import util.Constants;
import util.Errors;
import util.LaptopConstants;
import util.MobileConstants;

import com.esotericsoftware.kryonet.Connection;

import db.Sqllitedb;


public class Registration {
	private Logger logger = LoggerFactory.getLogger(Registration.class);
	
	public SomeResponse doMobileRegistration(String username){
		SomeResponse response;
		String keycode = null;
		boolean result = false;
		try {
			Sqllitedb dbInstance = Sqllitedb.getInstance();
			result = dbInstance.checkAccountExists(username);
			if(result == false){
			    keycode = Constants.randomString();
				dbInstance.createAccount(username, keycode);
				
			}else{
				response = Errors.generateError(Errors.USER_ALREADY_EXISTS);
			}
			result = true;
				
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			result = false;
		}
		if(result == true){
			response = new SomeResponse();
			response.command = MobileConstants.cmd_mobileRegistrationDone;
			response.data = keycode;
			logger.info("Mobile registered successfully =" + username);
		}
		else{
			response = Errors.generateError(Errors.TRY_AGAIN_LATER);
			logger.error("Mobile registered unsuccessfully =" + username);
		}
		return response;
	}
	
	public SomeResponse doLaptopRegistration(String username){
		SomeResponse response = null;
		String keycode = null;
		boolean result = false;
		try {
			Sqllitedb dbInstance = Sqllitedb.getInstance();
			result = dbInstance.checkLaptopAccountExists(username);
			if(result == true){
			    keycode = dbInstance.getKeyCode(username);	
			    result = dbInstance.createLaptopAccount(username);
			}else{
				response = Errors.generateError(Errors.USER_NOT_FOUND);
				logger.error("username not found during laptop registration = "+ username);
				return response;
			}
			result = true;
				
		} catch (Exception e) {
			e.printStackTrace();
			logger.error("Some error happened during laptop registration = "+ username);
			response = Errors.generateError(Errors.TRY_AGAIN_LATER);
		}
		if(result == true){
			response = new SomeResponse();
			response.command = LaptopConstants.cmd_laptopRegistrationDone;
			response.data = keycode;
			logger.info("laptop registered successfully");
		}
		return response;
	}
}
