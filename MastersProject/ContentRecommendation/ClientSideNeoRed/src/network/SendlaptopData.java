package network;

import java.io.File;
import java.io.FileFilter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Arrays;
import java.util.Date;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import Util.Constants;
import Util.Util;

import com.esotericsoftware.kryo.Kryo;
import com.esotericsoftware.kryonet.Client;
import com.esotericsoftware.kryonet.Connection;
import com.esotericsoftware.kryonet.Listener;

public class SendlaptopData {
	private Client client = null;
    private  boolean sent = true;
    private  boolean done = false;
	private Logger logger = LoggerFactory.getLogger(SendlaptopData.class);
	
	public boolean sendLaptopDataLinks(){
		boolean someDownloaded = false;
		String username = Util.getPreferences(Constants.pref_username);
		String folderToSend = findNextFolderToShare();
		if(username == null)
			return false;
		while(sent){
			if(folderToSend!=null){
                String jsonData = Util.readFile(System.getProperty("user.dir") + File.separator + Constants.dir_links
                		+ File.separator + folderToSend + File.separator + Constants.file_chrome);
                sent = sendDataToServer(folderToSend, jsonData);
			}
			else{
				sent = true;
				break;
			}
			folderToSend = findNextFolderToShare();
		}
		return someDownloaded;
	}	 
	 boolean sendDataToServer(final String date, final String jsonData){
		 		sent = false;
		        done = false;
		        final String username = Util.getPreferences(Constants.pref_username);
		        final String keycode = Util.getPreferences(Constants.pref_keycode);
		        client = new Client(Constants.READ_BUFFER,Constants.WRITE_BUFFER);
		        com.esotericsoftware.minlog.Log.set(com.esotericsoftware.minlog.Log.LEVEL_DEBUG);
		        Kryo kryo = client.getKryo();
		        kryo.register(SomeRequest.class);
		        kryo.register(SomeResponse.class);
		        client.addListener(new Listener() {
		            public void connected(Connection connection) {
		                SomeRequest request = new SomeRequest();
		                request.command = Constants.cmd_SendlaptopData;
		                request.user = username;
		                request.keycode = keycode;
		                request.data = jsonData;
		                request.date = date;
		                request.sender = Constants.laptop;
		                client.sendTCP(request);
		            }

		            public void received(Connection connection, Object object) {
		            	logger.info("Send laptop data response received from server");
		                if (object instanceof SomeResponse) {
		                    SomeResponse response = (SomeResponse) object;
		                    if(response.command.equals(Constants.cmd_SendlaptopDataDone)){   
		                        		sent = true;
		                                Util.setPreferences(Constants.pref_dateShareLaptopData, date);
		                                logger.info("Send laptop data response received from server for date ="+response.date);
		                    }       
		                    else{
		                   //Stop receivng data finished
		                    	sent = false;
		                    }
		                    done = true;
		               }
		            }

		            public void disconnected(Connection connection) {
		                done = true;
		                sent = false;
		                logger.error("Send laptop data response not received for date ="+ date);
		            }
		        });
		        new Thread(client).start();
		        try {
		            client.connect(Constants.connection_duration, Constants.SERVER_IP, Constants.port1,
		                    Constants.port2);
		        } catch (IOException e) {
		            // TODO Auto-generated catch block
		        	logger.error("Exception during Send laptop data response data not received for date ="+ e.toString());
		        	done = true;
		        }
		        while(!done);

		        return sent;
		    }
	 
	 public  String findNextFolderToShare() {

	        String lastFolder = Util.getPreferences(Constants.pref_dateShareLaptopData);
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
	                String topFolder = null;
		            topFolder = files[files.length - 1].getName() ;
		            //Yesterday folder was last sent
		            if(lastFolder != null && lastFolder.equals(yesterDay)){
		                return null;
		            }
		            else if(topFolder != null && lastFolder.equals(topFolder)){
		                return null;
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
}
