import java.awt.Desktop;
import java.io.File;
import java.io.IOException;

import network.DownloadAllData;
import network.GetmobileData;
import network.Registration;
import network.SendFeedback;
import network.SendRecommendations;
import network.SendlaptopData;

import org.quartz.Job;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import Util.ChromeJson;
import Util.Constants;
import Util.DownloadLinks;


public class Handler implements Job{
	private Logger logger = LoggerFactory.getLogger(Handler.class);
	public void execute(JobExecutionContext context)
            throws JobExecutionException {
                logger.info("Scheduler"); 
                Boolean status = false;
                
                //Check for registration
                Registration regObj = new Registration();
                status = regObj.doLaptopRegistration();
                if(status == true){
                	logger.info("laptop user registered");
                }
                else{
                	logger.error("Laptop software returned because of lack of registration");
                	return;
                }
                
                //Get all chromeJson data of laptop
               ChromeJson chromeObj = new ChromeJson();
               chromeObj.createChromeJson();
                
               //Get data extracted from mobile from server
                GetmobileData mobileObj = new GetmobileData();
                status = mobileObj.getMobileDataLinks();
                if(status == true){
                	logger.info("Mobile data extracted from server properly");
                }
                else{
                	logger.error("Mobile data extracted from server improperly");
                }
                
                //Download mobile data
                DownloadAllData dwdObj = new DownloadAllData();
                dwdObj.downloadEverything();
                
                //Send laptop data to server
                SendlaptopData laptopObj = new SendlaptopData();
                status = laptopObj.sendLaptopDataLinks();
                if(status == true){
                	logger.info("Laptop data sent to server properly");
                }
                else{
                	logger.error("Laptop data sent to server improperly");
                }
                
              //Send laptop recommendations to server
                SendRecommendations recommendObj = new SendRecommendations();
                status = recommendObj.sendRecommendationsLinks();
                if(status == true){
                	logger.info("Laptop recommendations sent to server properly");
                }
                else{
                	logger.error("Laptop recommendations sent to server improperly");
                }
                
                //Send laptop feedback to server
                SendFeedback feedbackObj = new SendFeedback();
                status = feedbackObj.sendFeedback();
                if(status == true){
                	logger.info("Laptop feedback sent to server properly");
                }
                else{
                	logger.error("Laptop feedback sent to server improperly");
                }
                
                logger.info("Time to show recommendations");
                ShowRecommendations showRecommObj = new ShowRecommendations();
                showRecommObj.showScreen();
                logger.info("Time to run python prediction app");
                try {
					Desktop.getDesktop().open(new File("neoredLaptop.app"));
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
            }
}
