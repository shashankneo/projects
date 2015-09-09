package com.example.neocfc.activity;


import android.app.Activity;
import android.app.Fragment;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.FragmentActivity;
import android.os.Bundle;
import android.util.Base64;
import android.util.Log;
import android.view.MenuItem;

import com.example.neocfc.async.ChromeAsync;
import com.example.neocfc.neored.R;
import com.example.neocfc.util.Constants;
import com.example.neocfc.util.SharedPreferencesUtil;
import com.example.neocfc.util.Util;
import com.facebook.CallbackManager;
import com.facebook.FacebookSdk;
import com.facebook.login.widget.LoginButton;

import com.twitter.sdk.android.Twitter;
import com.twitter.sdk.android.core.TwitterAuthConfig;
import io.fabric.sdk.android.Fabric;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import android.content.pm.Signature;
import java.util.Date;


public class MainActivity extends FragmentActivity{

    // Note: Your consumer key and secret should be obfuscated in your source code before shipping.
    private static final String TWITTER_KEY = "i40fO7mMhRHFed8uxd588lDFL";
    private static final String TWITTER_SECRET = "sxQPYSqRTHTEmCf79ImwVXkD7KjL2AWx7csjkwmD40pDVLFGYv";
    /*private Handler mHandler = null;
    private ChromeAsync chromeAsync = null;*/
    private String TAG = "MainActivity";
    private Activity activity = null;
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        activity = this;
        SharedPreferencesUtil.setStoredPref(activity.getApplicationContext(), Constants.MAIN_ACTIVITY,
                "yes");
        TwitterAuthConfig authConfig = new TwitterAuthConfig(TWITTER_KEY, TWITTER_SECRET);
        Fabric.with(this, new Twitter(authConfig));
        FacebookSdk.sdkInitialize(getApplicationContext());
        Log.v(TAG,"Main activity hit up");

        try {
            PackageInfo info = getPackageManager().getPackageInfo(
                    "com.example.neocfc.neored",
                    PackageManager.GET_SIGNATURES);
            for (Signature signature : info.signatures) {
                MessageDigest md = MessageDigest.getInstance("SHA");
                md.update(signature.toByteArray());
                Log.v(TAG,"KeyHash=" + Base64.encodeToString(md.digest(), Base64.DEFAULT));
            }
        } catch (PackageManager.NameNotFoundException e) {

        } catch (NoSuchAlgorithmException e) {

            }

        Util.startUpKit(activity.getApplicationContext());
        //Move handler in startupkit later to do...
        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            public void run() {
                while(FacebookSdk.isInitialized() == false);
                Util.startBackgroundDaemons(getApplicationContext());
            }
        }, 5000);

        setContentView(R.layout.activity_main);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        // Pass the activity result to the fragment, which will
        // then pass the result to the login button.
        Fragment fragment = getFragmentManager()
                .findFragmentById(R.id.twitterfragment);
        if (fragment != null) {
            fragment.onActivityResult(requestCode, resultCode, data);
        }
    }
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
    @Override
    protected void onStop() {
        SharedPreferencesUtil.setStoredPref(activity.getApplicationContext(), Constants.MAIN_ACTIVITY,
                "no");
        super.onPause();
    }
}
