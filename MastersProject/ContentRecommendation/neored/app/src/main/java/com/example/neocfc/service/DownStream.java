package com.example.neocfc.service;

import android.app.IntentService;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Environment;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

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

import java.io.IOException;
import java.net.InetAddress;
import java.util.Date;

public class DownStream extends IntentService {


    private static final String TAG = "Downstream";
    private Client client = null;
    private  boolean recv = false;
    private  boolean done = false;
    private SomeResponse chkResponse = null;

    public DownStream() {
        super(DownStream.class.getName());

    }

    public int onStartCommand(Intent intent, int flags, int startId) {

        return super.onStartCommand(intent,flags,startId);
    }
    @Override
    protected void onHandleIntent(Intent intent) {
        boolean result = true;
        Log.v(TAG, "Downstream Started!");
        String folderToRecieve = SharedPreferencesUtil.getStoredPref(getApplicationContext(),
                Constants.folderRecieve);
        if(folderToRecieve != null && folderToRecieve.equals(Util.getDateString(new Date()))){
            return;
            }
        while(result){
            result = receiveDataFromServer(folderToRecieve);
            folderToRecieve = SharedPreferencesUtil.getStoredPref(getApplicationContext(),
                    Constants.folderRecieve);
        }

        //turn the downtream alarm off because either all folders have been received
       // if(chkResponse.date==null){
       //     Util.cancelDownAlarm(getApplicationContext());
       // }
    }

    boolean receiveDataFromServer(final String date){
        recv = false;
        done = false;
        final String username = SharedPreferencesUtil.getStoredPref(getApplicationContext(), Constants.username);
        final String keycode = SharedPreferencesUtil.getStoredPref(getApplicationContext(), Constants.key);
        client = new Client(Constants.READ_BUFFER,Constants.WRITE_BUFFER);
        com.esotericsoftware.minlog.Log.set(com.esotericsoftware.minlog.Log.LEVEL_TRACE);
        Kryo kryo = client.getKryo();
        kryo.register(SomeRequest.class);
        kryo.register(SomeResponse.class);
        client.addListener(new Listener() {
            public void connected(Connection connection) {
                android.util.Log.v(TAG, "Mobile Client connected for receiving recommend data="+date);
                SomeRequest request = new SomeRequest();
                request.command = MobileConstants.cmd_RecvmobileRecomm;
                request.user = username;
                request.keycode = keycode;
                request.date = date;
                request.sender = MobileConstants.mobile;
                client.sendTCP(request);
            }

            public void received(Connection connection, Object object) {
                android.util.Log.v(TAG, "Mobile Client received recomm from sever");
                if (object instanceof SomeResponse) {
                    SomeResponse response = (SomeResponse) object;
                    if(response.command.equals(MobileConstants.cmd_RecvmobileRecommDone)){
                        android.util.Log.v(TAG, "mobile Client json successfully received recomm at " +
                                "client ");
                        if(response.date!=null){
                            recv = Util.writeRecommJson(response.date,response.data);
                            if(recv == true) {
                                SharedPreferencesUtil.setStoredPref(getApplicationContext(),
                                        Constants.folderRecieve, response.date);
                            }
                        }
                        else{
                            //Stop receivng data finished
                            recv = false;
                        }
                    }
                    else if(response.command == Constants.errors){
                        android.util.Log.v(TAG, "mobile Client error json successfully received " +
                                "recomm at server ");
                        recv = false;
                    }
                    done = true;
                }
            }

            public void disconnected(Connection connection) {
                done = true;
                recv = false;
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

        return recv;
    }
}