import org.apache.log4j.PropertyConfigurator;

import java.awt.Desktop;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;

import org.quartz.JobDetail;
import org.quartz.Scheduler;
import org.quartz.SchedulerException;
import org.quartz.Trigger;
import org.quartz.impl.StdSchedulerFactory;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import Util.ChromeJson;
import Util.Constants;
import Util.DownloadLinks;
import Util.Util;
import static org.quartz.JobBuilder.newJob;
import static org.quartz.TriggerBuilder.newTrigger;
import static org.quartz.SimpleScheduleBuilder.simpleSchedule;
public class Client {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		// TODO Auto-generated method stub
				/*ChromeJson obj = new ChromeJson();
				obj.createChromeJson(new Date());*/
				Util.createLogFile();
				PropertyConfigurator.configure("log4j.properties");
				Logger logger = LoggerFactory.getLogger(Client.class);
//				try {
//					Desktop.getDesktop().browse(new URI("http://www.google.com"));
//				} catch (IOException e) {
//					// TODO Auto-generated catch block
//					e.printStackTrace();
//				} catch (URISyntaxException e) {
//					// TODO Auto-generated catch block
//					e.printStackTrace();
//				}
				logger.info("neored Client side application launch");
				try {	
		            // Grab the Scheduler instance from the Factory 
		            Scheduler scheduler = StdSchedulerFactory.getDefaultScheduler();
		            // and start it off
		            scheduler.start();
		            
		         
		            JobDetail job1 = newJob(Handler.class)
		                .withIdentity("job1", "group1")
		                .build();
		            Trigger trigger1 = newTrigger()
		                .withIdentity("trigger1", "group1")
		                .startNow()
		                .withSchedule(simpleSchedule()
		                        .withIntervalInMinutes(120)
		                        .repeatForever())            
		                .build();
		            
		            scheduler.scheduleJob(job1, trigger1);
		 
		            //scheduler.shutdown();

		        } catch (SchedulerException se) {
		            se.printStackTrace();
		        }
	}

}

