package com.example.neocfc.service;

import android.app.IntentService;
import android.content.Intent;
import android.util.Log;

import com.example.neocfc.util.Constants;
import com.example.neocfc.util.JsonUtil;
import com.example.neocfc.util.SharedPreferencesUtil;
import com.example.neocfc.util.Util;
import com.facebook.FacebookSdk;
import com.facebook.LoggingBehavior;

import org.json.JSONObject;

/**
 * Created by neocfc on 6/30/15.
 */
public class WorkerService  extends IntentService {


    private static final String TAG = "WorkerService";

    public WorkerService() {
        super(WorkerService.class.getName());

    }

    public int onStartCommand(Intent intent, int flags, int startId) {

        return super.onStartCommand(intent,flags,startId);
    }
    @Override
    protected void onHandleIntent(Intent intent) {

        Log.v(TAG, "WorkerService Started!");
        JsonUtil objJsonUtil = new JsonUtil();
        //Create today's date folder
        boolean fb_json = false;
        boolean twitter_json = false;
        boolean chrome_json = false;
        boolean ultimate_json = false;
        boolean sent_to_MainActivity = false;
        String mainActivity = SharedPreferencesUtil.getStoredPref(getApplicationContext(),
                Constants.MAIN_ACTIVITY);
        if(mainActivity!=null && mainActivity.equals("yes")){
            sent_to_MainActivity = true;
        }
        //Checks for todays folder
        if((ultimate_json = Util.checkUltimateJson())==false) {
            Util.createNewMobileDateFolder();
            //chrome
            if ((chrome_json = Util.checkChromeJson()) == false) {
                Log.v(TAG, " Chrome json not made for today. Go and prepare");
                chrome_json = objJsonUtil.getChromeJson(getApplicationContext());
            } else {
                Log.v(TAG, " Chrome json made for today. Awesome!!");
            }

            //facebook
            if ((fb_json = Util.checkFbJson()) == false) {
                Log.v(TAG, " Fb json not made for today. Go and prepare");
                //Check whether fb is logged in
                if (Util.isFbLoggedIn(getApplicationContext()) == false) {
                    //Go to for user to provide login credentials
                    Log.v(TAG, " Fb user is not logged in. Go to Main Activity");
                    if(sent_to_MainActivity!=true){
                        Util.goToMainActivity(getApplicationContext());
                        sent_to_MainActivity = true;
                    }

                } else {
                    fb_json = objJsonUtil.getFbJson();
                }
            } else {
                Log.v(TAG, " Fb json made for today. Awesome!!");
            }

            //Twitter
            if ((twitter_json = Util.checkTwitterJson()) == false) {
                Log.v(TAG, " Twitter json not made for today. Go and prepare");
                //Check whether twitter is logged in
                if (Util.isTwitterLoggedIn(getApplicationContext()) == false) {
                    //Go to for user to provide login credentials
                    Log.v(TAG, " Twitter user is not logged in. Go to Main Activity");
                    if(sent_to_MainActivity!=true)
                        Util.goToMainActivity(getApplicationContext());
                } else {
                    twitter_json = objJsonUtil.getTwitterJson();
                }
            } else {
                Log.v(TAG, " Twitter json made for today. Awesome!!");
            }

            if (chrome_json == true && fb_json == true && twitter_json == true) {
                //Create the ultimate file to be shared
                ultimate_json = objJsonUtil.createUltimateFile();
            }
        }

        if(ultimate_json == true){
            //Try to connect to Server
        }
    }
}