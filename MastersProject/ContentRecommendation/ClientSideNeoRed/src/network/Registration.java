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
import java.io.IOException;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.*;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import Util.AndroidLikeToast;
import Util.ChromeJson;
import Util.Constants;
import Util.Util;

import com.esotericsoftware.kryo.Kryo;
import com.esotericsoftware.kryonet.Client;
import com.esotericsoftware.kryonet.Connection;
import com.esotericsoftware.kryonet.Listener;
import com.esotericsoftware.minlog.Log;


public class Registration {
	boolean result = false;
	boolean done = false;
	Client client;
	private Logger logger = LoggerFactory.getLogger(Registration.class);
	
	public boolean doLaptopRegistration(){
		String username = Util.getPreferences(Constants.pref_username);
		if(username != null)
			return true;
		LoginDialog ld = new LoginDialog();
		ld.setSize(400, 150);
		ld.setVisible(true);
		while(!done);
		return result;
	}
	
	public class LoginDialog extends JDialog {
		 
		JLabel nameLabel = new JLabel("Name : ");
		JTextField nameField = new JTextField();
	 
		JButton okButton = new JButton("OK");
		JButton cancelButton = new JButton("Cancel");
	 
		public LoginDialog() {
			setupUI();
	 
			setUpListeners();
	 
		}
	 
		public void setupUI() {
	 
			this.setTitle("Login");
	 
			JPanel topPanel = new JPanel(new GridBagLayout());
			JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT));
	 
			buttonPanel.add(okButton);
			buttonPanel.add(cancelButton);
	 
			GridBagConstraints gbc = new GridBagConstraints();
	 
			gbc.insets = new Insets(4, 4, 4, 4);
	 
			gbc.gridx = 0;
			gbc.gridy = 0;
			gbc.weightx = 0;
			topPanel.add(nameLabel, gbc);
	 
			gbc.gridx = 1;
			gbc.gridy = 0;
			gbc.fill = GridBagConstraints.HORIZONTAL;
			gbc.weightx = 1;
			topPanel.add(nameField, gbc);

	 
			this.add(topPanel);
	 
			this.add(buttonPanel, BorderLayout.SOUTH);
	 
		}
	 
		private void setUpListeners() {
	 
			nameField.addKeyListener(new KeyAdapter() {
	 
				@Override
				public void keyPressed(KeyEvent e) {
					if (e.getKeyCode() == KeyEvent.VK_ENTER) {
						login();
					}
				}
			});
	 
			okButton.addActionListener(new ActionListener() {
	 
				@Override
				public void actionPerformed(ActionEvent e) {
					login();
				}
			});
	 
			cancelButton.addActionListener(new ActionListener() {
	 
				@Override
				public void actionPerformed(ActionEvent e) {
					LoginDialog.this.setVisible(false);
					done = true;
				}
			});
		}
		private void login() {
			System.out.println("You logged in!");
			LoginDialog.this.setVisible(false);
			connectToServer(nameField.getText());
		}
		
		 private void connectToServer(final String username){
		        result = false;
		        done = false;
		        client = new Client();
		        Log.set(Log.LEVEL_DEBUG);
		        Kryo kryo = client.getKryo();
		        kryo.register(SomeRequest.class);
		        kryo.register(SomeResponse.class);
		        client.addListener(new Listener() {
		            public void connected(Connection connection) {
		                logger.info("Laptop connected to server for user registration"+username);
		                SomeRequest request = new SomeRequest();
		                request.command = Constants.cmd_laptopRegistration;
		                request.user = username;
		                request.sender = Constants.laptop;
		                client.sendTCP(request);
		            }

		            public void received(Connection connection, Object object) {
		                logger.info("Registration response received from server");
		                if (object instanceof SomeResponse) {
		                    SomeResponse response = (SomeResponse) object;
		                    if(response.command.equals(Constants.cmd_laptopRegistrationDone)){
		                        logger.info("laptp successfully registered with server="+username);
		                        String keycode = response.data;
		                        result = true;
		                        Util.setPreferences(Constants.pref_username, username);
		                        Util.setPreferences(Constants.pref_keycode, keycode);
		                        ToastDemo e = new ToastDemo("User successfully registered");
		                        e.setVisible(true);
		                    }
		                    else{
		                    	logger.info("Problem with user registration ="+username);
		                    	ToastDemo e = new ToastDemo("Problem with user registration");
		                        e.setVisible(true);
		                    }
		                    done = true;
		                }
		            }

		            public void disconnected(Connection connection) {
		                done = true;
		                logger.error("Laptop disconnected from server for some reason");
		                ToastDemo e = new ToastDemo("Problem with user registration");
                        e.setVisible(true);
		            }
		        });
		        new Thread(client).start();
		        try {
		            client.connect(Constants.connection_duration, Constants.SERVER_IP, Constants.port1,
		                    Constants.port2);
		        } catch (IOException e) {
		            // TODO Auto-generated catch block
		           logger.error("Exception while registering laptop to sever ="+e.toString());
		           done = true;
		        }
		        while(!done);
		    }
	}
	
	public class ToastDemo extends JFrame {
	    public ToastDemo(String msg) {
	        super(msg);
	        setSize(400, 300);
	        setLayout(new FlowLayout());
	        setDefaultCloseOperation(EXIT_ON_CLOSE);
	        final JDialog dialog = new AndroidLikeToast(ToastDemo.this, true, msg);
            Timer timer = new Timer(AndroidLikeToast.LENGTH_LONG, new ActionListener() {

                @Override
                public void actionPerformed(ActionEvent e) {
                    dialog.setVisible(false);
                    dialog.dispose();
                }
            });
            timer.setRepeats(false);
            timer.start();

            dialog.setVisible(true); // if modal, application will pause here
	    }
	}
}
	
