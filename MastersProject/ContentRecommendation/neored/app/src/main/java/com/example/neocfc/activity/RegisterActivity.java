package com.example.neocfc.activity;

import android.app.Activity;
import android.content.Intent;
import android.os.Message;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

import com.example.neocfc.async.RegisterAsync;
import com.example.neocfc.neored.R;
import com.example.neocfc.util.Constants;
import com.example.neocfc.util.JsonData;
import com.example.neocfc.util.SharedPreferencesUtil;
import com.example.neocfc.util.Util;
import com.facebook.FacebookSdk;

import android.os.Handler;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class RegisterActivity extends ActionBarActivity {
    private Handler mHandler = null;
    private RegisterAsync registerAsync = null;
    private Button btnRegister;
    private EditText txtUser;
    private Activity activity;
    private String TAG ="RegisterActivity";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_register);
        activity = this;
        btnRegister = (Button) findViewById(R.id.btnRegister);
        txtUser = (EditText)findViewById(R.id.username);

        btnRegister.setOnClickListener(new View.OnClickListener() {

            public void onClick(View v) {
                mHandler = new Handler() {
                    public void handleMessage(Message msg) {
                        String registration = (String) msg.obj;
                        Log.v(TAG, "Handler handle message=" + JsonData.toast_msg);
                        Toast.makeText(activity,JsonData.toast_msg, Toast.LENGTH_SHORT).show();
                        if(registration == "yes"){
                            Util.goToMainActivity(getApplicationContext());
                        }
                    }
                };
                String userName = txtUser.getText().toString();
                if (userName == null || userName.isEmpty()) {
                    Toast.makeText(activity, "Please enter username", Toast.LENGTH_SHORT).show();
                }
                registerAsync = new RegisterAsync(getApplicationContext(), mHandler);
                registerAsync.execute(userName);

            }
        });
        checkWhetherRegistered();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_register, menu);
        return true;
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
    public void checkWhetherRegistered()
    {
        String key = SharedPreferencesUtil.getStoredPref(this, Constants.key);
        if(key != null)
        {
            //User registered. Transfer to Recommendations Page
            Util.startUpKit(activity.getApplicationContext());
            Handler handler = new Handler();
            handler.postDelayed(new Runnable() {
                public void run() {
                    while (FacebookSdk.isInitialized() == false) ;
                    Util.startBackgroundDaemons(getApplicationContext());
                }
            }, 5000);
            Util.goToRecommendationActivity(getApplicationContext());

        }
    }
}
