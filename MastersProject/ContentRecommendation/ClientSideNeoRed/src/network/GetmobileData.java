package network;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Date;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.Timer;

import network.Registration.LoginDialog;
import network.Registration.ToastDemo;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import Util.AndroidLikeToast;
import Util.Constants;
import Util.Util;

import com.esotericsoftware.kryo.Kryo;
import com.esotericsoftware.kryonet.Client;
import com.esotericsoftware.kryonet.Connection;
import com.esotericsoftware.kryonet.Listener;
import com.esotericsoftware.minlog.Log;

public class GetmobileData {
	private Client client = null;
    private  boolean recv = true;
    private  boolean done = false;
	private Logger logger = LoggerFactory.getLogger(GetmobileData.class);
	
	public boolean getMobileDataLinks(){
		boolean someDownloaded = false;
		String username = Util.getPreferences(Constants.pref_username);
		String folderToRecieve = Util.getPreferences(Constants.pref_dateUltimateLinks);
		if(folderToRecieve != null && folderToRecieve.equals(Util.getDateString(new Date()))){
			return true;
		}
		if(username == null)
			return false;
		while(recv){
			recv = receiveDataFromServer(folderToRecieve);
			folderToRecieve = Util.getPreferences(Constants.pref_dateUltimateLinks);
			if(recv)
				someDownloaded = true;
		}
		return someDownloaded;
	}	 
	 boolean receiveDataFromServer(final String date){
		        recv = false;
		        done = false;
		        final String username = Util.getPreferences(Constants.pref_username);
		        final String keycode = Util.getPreferences(Constants.pref_keycode);
		        client = new Client(Constants.READ_BUFFER,Constants.WRITE_BUFFER);
		        com.esotericsoftware.minlog.Log.set(com.esotericsoftware.minlog.Log.LEVEL_DEBUG);
		        com.esotericsoftware.minlog.Log.TRACE();
		        Kryo kryo = client.getKryo();
		        kryo.register(SomeRequest.class);
		        kryo.register(SomeResponse.class);
		        client.addListener(new Listener() {
		            public void connected(Connection connection) {
		                SomeRequest request = new SomeRequest();
		                request.command = Constants.cmd_RecvlaptopData;
		                request.user = username;
		                request.keycode = keycode;
		                request.date = date;
		                request.sender = Constants.laptop;
		                client.sendTCP(request);
		            }

		            public void received(Connection connection, Object object) {
		            	
		                if (object instanceof SomeResponse) {
		                	logger.info("Download Mobile data response received from server");
		                    SomeResponse response = (SomeResponse) object;
		                    if(response.command.equals(Constants.cmd_RecvlaptopDataDone)){   
		                        if(response.date!=null){
		                            recv = writeRecommJson(response.date,response.data);
		                            if(recv == true) {
		                                Util.setPreferences(Constants.pref_dateUltimateLinks, response.date);
		                                logger.info("Download mobile data received for date ="+response.date);
		                            }
		                        }
		                        else{
		                            //Stop receivng data finished
		                            recv = false;
		                        }
		                        done = true;
		                    }
		                   		                }
		            }

		            public void disconnected(Connection connection) {
		                done = true;
		                recv = false;
		                logger.error("Download mobile data not received for date ="+ date);
		            }
		        });
		        new Thread(client).start();
		        try {
		            client.connect(Constants.connection_duration, Constants.SERVER_IP, Constants.port1,
		                    Constants.port2);
		        } catch (IOException e) {
		            // TODO Auto-generated catch block
		        	logger.error("Exception during Download mobile data not received for date ="+ e.toString());
		        	done = true;
		        }
		        while(!done);

		        return recv;
		    }
		 public boolean writeRecommJson(String date, String jsonString) {
			 	if(jsonString==null)
			 		return true;
		        boolean result = false;
		        File folder = new File(System.getProperty("user.dir") + File.separator + Constants.dir_links
		                + File.separator+ date);
		        if(folder.exists()==false){
		            folder.mkdirs();
		        }
		        File f = new File(System.getProperty("user.dir")+ File.separator + Constants.dir_links +
		        		File.separator+ date, Constants.file_ultimate);
		        FileWriter file = null;
		        try {
		            f.createNewFile();
		            file = new FileWriter(f);
		            file.write(jsonString);
		            logger.info("Successfully written ultimate.json for ="+date);
		            result = true;
		        } catch (IOException e) {
		            logger.error("Unsuccessfully written ultimate json for ="+date);

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
		 
}
