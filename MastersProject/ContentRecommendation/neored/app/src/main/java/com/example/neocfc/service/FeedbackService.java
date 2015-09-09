package com.example.neocfc.service;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.os.Environment;
import android.util.Log;

import com.esotericsoftware.kryo.Kryo;
import com.esotericsoftware.kryonet.Client;
import com.esotericsoftware.kryonet.Connection;
import com.esotericsoftware.kryonet.Listener;
import com.example.neocfc.com.example.neocfc.network.SomeRequest;
import com.example.neocfc.com.example.neocfc.network.SomeResponse;
import com.example.neocfc.util.Constants;
import com.example.neocfc.util.MobileConstants;
import com.example.neocfc.util.SharedPreferencesUtil;
import com.example.neocfc.util.Util;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Date;
import java.io.FileFilter;

public class FeedbackService  extends IntentService {


    private static final String TAG = "FeedbackService";
    private  Client client = null;
    private  boolean sent = false;
    private  boolean done = false;
    public FeedbackService() {
        super(FeedbackService.class.getName());

    }

    public int onStartCommand(Intent intent, int flags, int startId) {

        return super.onStartCommand(intent,flags,startId);
    }
    @Override
    protected void onHandleIntent(Intent intent) {
        boolean result = true;
        Log.v(TAG, "FeedbackService Started!");
        String folderToSent = findNextFolderToShare(getApplicationContext());
        while(result){

            if(folderToSent!=null){
                String jsonData = Util.readFile(Environment.getExternalStorageDirectory() + File.separator +
                        Constants.DIR + File.separator + Constants.laptop + File.separator +
                        folderToSent+ File.separator + Constants.Ultimate_file);
                result = sendDataToServer(folderToSent,jsonData);
            }
            else{
                break;
            }
            folderToSent = findNextFolderToShare(getApplicationContext());
        }
        //TODO handle the alarm(cancelling)
        //turn the upstream alarm off because either result is true(all info is sent)
        //or folderToSent is null(nothing to send)
        //  Util.cancelUpAlarm(getApplicationContext());
    }

    boolean sendDataToServer(final String date, final String jsonData){
        sent = false;
        final String username = SharedPreferencesUtil.getStoredPref(getApplicationContext(), Constants.username);
        final String keycode = SharedPreferencesUtil.getStoredPref(getApplicationContext(), Constants.key);
        client = new Client(Constants.READ_BUFFER,Constants.WRITE_BUFFER);
        com.esotericsoftware.minlog.Log.set(com.esotericsoftware.minlog.Log.LEVEL_DEBUG);
        Kryo kryo = client.getKryo();
        kryo.register(SomeRequest.class);
        kryo.register(SomeResponse.class);
        client.addListener(new Listener() {
            public void connected(Connection connection) {
                android.util.Log.v(TAG, "Mobile Client connected for sending feedback data=" + date);
                SomeRequest request = new SomeRequest();
                request.command = MobileConstants.cmd_SendmobileFeedback;
                request.user = username;
                request.keycode = keycode;
                request.data = jsonData;
                request.date = date;
                request.sender = MobileConstants.mobile;
                client.sendTCP(request);
            }

            public void received(Connection connection, Object object) {
                android.util.Log.v(TAG, "Mobile Client received message from sever after sending feedback ");
                if (object instanceof SomeResponse) {
                    SomeResponse response = (SomeResponse) object;
                    if (response.command.equals(MobileConstants.cmd_SendmobileFeedbackDone)) {
                        android.util.Log.v(TAG, "mobile Client feedback successfully received at server ");
                        sent = true;
                        SharedPreferencesUtil.setStoredPref(getApplicationContext(),
                                Constants.feedbackShared, date);
                    } else if (response.command == Constants.errors) {
                        android.util.Log.v(TAG, "mobile Client feedback received at server ");
                        sent = false;
                    }
                    done = true;
                }
            }

            public void disconnected(Connection connection) {
                done = true;
                sent = false;
                android.util.Log.v(TAG, "Client disconnected from server during sending json ");
            }
        });
        new Thread(client).start();
        try {
            client.connect(Constants.connection_duration, Constants.SERVER_IP, Constants.port1,
                    Constants.port2);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            android.util.Log.v(TAG,"Exception during registration to server ="+e.toString());
            done = true;
        }
        while(!done);

        return sent;
    }

    public  String findNextFolderToShare(Context mContext) {

        String lastFolder = SharedPreferencesUtil.getStoredPref(mContext, Constants.feedbackShared);
        File f = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR+
                File.separator+Constants.laptop);
        String yesterDay = Util.getDateString(new Date(System.currentTimeMillis() -
                1000L * 60L * 60L * 24L));

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
            if(lastFolder == null){
                if(files.length == 1)
                    return null;
                else
                    return files[0].getName();
            }
            String topFolder = null;
            if(files.length >1 )
                topFolder = files[files.length - 1].getName() ;
            //Yesterday folder was last sent
            if(lastFolder != null && lastFolder.equals(yesterDay)){
                return null;
            }
            else if(topFolder != null && lastFolder.equals(topFolder)){
                return null;
            }
            else {
                int count = 0;
                for (File obj : files) {
                    count = count + 1;
                    if (obj.getName().equals(lastFolder)) {
                        break;
                    }
                }
                if (count != files.length) {
                    lastFolder = files[count].getName();
                } else {
                    lastFolder = null;
                }
            }
        }
        return lastFolder;
    }
}