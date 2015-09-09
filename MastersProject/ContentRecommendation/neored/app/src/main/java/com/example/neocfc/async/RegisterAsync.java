package com.example.neocfc.async;

/**
 * Created by neocfc on 6/28/15.
 */
import android.content.Context;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
import java.io.IOException;
import com.esotericsoftware.minlog.Log;


import com.esotericsoftware.kryo.Kryo;
import com.esotericsoftware.kryonet.Client;
import com.esotericsoftware.kryonet.Connection;
import com.esotericsoftware.kryonet.Listener;
import com.example.neocfc.com.example.neocfc.network.SomeRequest;
import com.example.neocfc.com.example.neocfc.network.SomeResponse;
import com.example.neocfc.util.Constants;
import com.example.neocfc.util.JsonData;
import com.example.neocfc.util.MobileConstants;
import com.example.neocfc.util.SharedPreferencesUtil;


public class RegisterAsync extends AsyncTask<String, Void, String> {
    Context mContext;
    Handler mHandler;
    Client client;
    boolean done;
    boolean result;
    private String TAG="RegisterAsync";
    public RegisterAsync(Context context,Handler handler) {
        super();
        mContext = context;
        mHandler=handler;
    }
    @Override
    protected String doInBackground(String... username) {

        String temp = connectToServer(username[0]);
        return temp;
    }

    @Override
    protected void onPostExecute(String result) {
        android.util.Log.v(TAG,"in LoginUserTask postExecute");
        Message msg = Message.obtain();
        msg.obj=result;
        mHandler.sendMessage(msg);
        // textView.setText(result);
    }
    private String connectToServer(final String username){
        result = false;
        done = false;
        client = new Client();
        Log.set(Log.LEVEL_DEBUG);
        Kryo kryo = client.getKryo();
        kryo.register(SomeRequest.class);
        kryo.register(SomeResponse.class);
        client.addListener(new Listener() {
            public void connected(Connection connection) {
                android.util.Log.v(TAG, "Client connected for registration ");
                SomeRequest request = new SomeRequest();
                request.command = MobileConstants.cmd_mobileRegistration;
                request.user = username;
                request.sender = MobileConstants.mobile;
                client.sendTCP(request);
            }

            public void received(Connection connection, Object object) {
                android.util.Log.v(TAG, "Client received key from server for registration ");
                if (object instanceof SomeResponse) {
                    SomeResponse response = (SomeResponse) object;
                    if(response.command.equals(MobileConstants.cmd_mobileRegistrationDone)){
                        android.util.Log.v(TAG, "Client received key from server for registration ");
                        String keycode = response.data;
                        JsonData.toast_msg = "Registration done. Now register laptop too.";
                        result = true;
                        SharedPreferencesUtil.setStoredPref(mContext,Constants.key,keycode);
                        SharedPreferencesUtil.setStoredPref(mContext,Constants.username,username);
                    }
                    else if(response.command == Constants.errors){
                        JsonData.toast_msg = response.error;
                    }
                    done = true;
                }
            }

            public void disconnected(Connection connection) {
                done = true;
                android.util.Log.v(TAG, "Client disconnected from server during registration ");
                JsonData.toast_msg = "Client disconnected from server during registration";
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
        if(result)
            return "yes";
        else
            return "no";
    }
}