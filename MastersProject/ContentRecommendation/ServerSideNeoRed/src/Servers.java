import java.io.IOException;

import org.apache.log4j.PropertyConfigurator;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import tasks.Data;
import tasks.Recommendations;
import tasks.Registration;
import util.Constants;
import util.Errors;
import util.LaptopConstants;
import util.MobileConstants;
import util.Util;
import network.SomeRequest;
import network.SomeResponse;







import com.esotericsoftware.kryo.Kryo;
import com.esotericsoftware.kryonet.Connection;
import com.esotericsoftware.kryonet.Listener;
import com.esotericsoftware.kryonet.Server;
import com.esotericsoftware.minlog.Log;


public class Servers {
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		
		//Set up log file
		Util.createLogFile();
		PropertyConfigurator.configure("log4j.properties");
		final Logger logger = LoggerFactory.getLogger(Data.class);
		Log.set(Log.LEVEL_DEBUG);
		Server server = new Server(Constants.READ_BUFFER,Constants.WRITE_BUFFER);
		Kryo kryo = server.getKryo();
		kryo.register(SomeRequest.class);
		kryo.register(SomeResponse.class);
	    server.start();
	    try {
	    	System.out.println("Server listening");
			server.bind(Constants.port1, Constants.port2);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	    
	    server.addListener(new Listener() {
	        public void received (Connection connection, Object object) {
	           System.out.println("Server received a message");
	           if (object instanceof SomeRequest) {
	              SomeRequest request = (SomeRequest)object;
	              SomeResponse response = null; 
	             
	              //Check command exists, and then see the sender
	              if(request.command!=null){
	            	  if(request.sender.equals(MobileConstants.mobile)){
	            		  logger.info("Command received from mobile = "+request.command);
	            		  if(request.command.equals(MobileConstants.cmd_mobileRegistration)){
		            		  //Register username
		            		  if(request.user!=null){
		            			  Registration obj = new Registration();
		            			  response = obj.doMobileRegistration(request.user);
		            		  }
		            		  else{
		            			  response = Errors.generateError(Errors.USER_NOT_FOUND);
		            		  }
		            			
		            	  }
	            		  else if(request.command.equals(MobileConstants.cmd_SendmobileData)){
		            		  //Recieve mobile json
		            		  if(request.user!=null && request.keycode !=null){
		            			  Data obj = new Data();
		            			  response = obj.doSendMobileData(request.user,
		            					  request.keycode, request.data, request.date);
		            		  }
		            		  else{
		            			  response = Errors.generateError(Errors.USER_NOT_FOUND);
		            		  }
		            			
		            	  }
	            		  else if(request.command.equals(MobileConstants.cmd_RecvmobileRecomm)){
		            		  //Recieve request for mobile recommendation
		            		  if(request.user!=null && request.keycode !=null){
		            			  Recommendations obj = new Recommendations();
		            			  response = obj.doRecvMobileRecomm(request.user,
		            					  request.keycode, request.date);
		            		  }
		            		  else{
		            			  response = Errors.generateError(Errors.USER_NOT_FOUND);
		            		  }
	            		
		            			
		            	  }
	            		  else if(request.command.equals(MobileConstants.cmd_SendmobileFeedback)){
		            		  //laptop sends its feedback to server
		            		  if(request.user!=null && request.keycode !=null){
		            			  Recommendations obj = new Recommendations();
		            			  response = obj.doSendMobileFeedback(request.user,
		            					  request.keycode, request.data, request.date);
		            		  }
		            		  else{
		            			  response = Errors.generateError(Errors.USER_NOT_FOUND);
		            		  }
		            			
		            	  }
	            		  
	            	  }
	            	  
	            	  if(request.sender.equals(LaptopConstants.laptop)){
	            		  logger.info("Command received from laptop = "+request.command);
	            		  if(request.command.equals(LaptopConstants.cmd_laptopRegistration)){
		            		  //Register username
		            		  if(request.user!=null){
		            			  Registration obj = new Registration();
		            			  response = obj.doLaptopRegistration(request.user);
		            		  }
		            		  else{
		            			  response = Errors.generateError(Errors.USER_NOT_FOUND);
		            		  }
		            			
		            	  }
	            		  else if(request.command.equals(LaptopConstants.cmd_RecvlaptopData)){
		            		  //Recieve request from laptop for mobile data
		            		  if(request.user!=null && request.keycode !=null){
		            			  Data obj = new Data();
		            			  response = obj.doRecvLaptopUltimate(request.user,
		            					  request.keycode, request.date);
		            		  }
		            		  else{
		            			  response = Errors.generateError(Errors.USER_NOT_FOUND);
		            		  }
		            			
		            	  }
	            		  else if(request.command.equals(LaptopConstants.cmd_SendlaptopData)){
		            		  //laptop sends its data to server
		            		  if(request.user!=null && request.keycode !=null){
		            			  Data obj = new Data();
		            			  response = obj.doSendLaptopData(request.user,
		            					  request.keycode, request.data, request.date);
		            		  }
		            		  else{
		            			  response = Errors.generateError(Errors.USER_NOT_FOUND);
		            		  }
		            			
		            	  }
	            		  else if(request.command.equals(LaptopConstants.cmd_SendlaptopRecommendation)){
		            		  //laptop sends its recommendation to server
		            		  if(request.user!=null && request.keycode !=null){
		            			  Recommendations obj = new Recommendations();
		            			  response = obj.doSendLaptopRecommendation(request.user,
		            					  request.keycode, request.data, request.date);
		            		  }
		            		  else{
		            			  response = Errors.generateError(Errors.USER_NOT_FOUND);
		            		  }
		            			
		            	  }
	            		  else if(request.command.equals(LaptopConstants.cmd_SendlaptopFeedback)){
		            		  //laptop sends its feedback to server
		            		  if(request.user!=null && request.keycode !=null){
		            			  Recommendations obj = new Recommendations();
		            			  response = obj.doSendLaptopFeedback(request.user,
		            					  request.keycode, request.data, request.date);
		            		  }
		            		  else{
		            			  response = Errors.generateError(Errors.USER_NOT_FOUND);
		            		  }
		            			
		            	  }
	            		  
	            	  }
	            	  
	              }
	              //Send the response back to sender
	              connection.sendTCP(response);
	           }
	        }
	     });
	}
	

}
