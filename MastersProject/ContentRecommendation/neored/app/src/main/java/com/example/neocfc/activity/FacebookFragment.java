package com.example.neocfc.activity;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import com.example.neocfc.neored.R;
import com.example.neocfc.util.Util;
import com.facebook.AccessToken;
import com.facebook.AccessTokenTracker;
import com.facebook.CallbackManager;
import com.facebook.FacebookCallback;
import com.facebook.FacebookException;
import com.facebook.FacebookRequestError;
import com.facebook.GraphRequest;
import com.facebook.GraphResponse;
import com.facebook.HttpMethod;
import com.facebook.login.LoginResult;
import com.facebook.login.widget.LoginButton;

import org.json.JSONObject;

import java.util.Arrays;
import java.util.Date;
import java.util.Set;

/**
 * Created by neocfc on 6/15/15.
 */
public class FacebookFragment extends Fragment {
    private CallbackManager callbackManager;
    private LoginButton loginButton;
    private AccessTokenTracker mFacebookAccessTokenTracker;
    private AccessToken accessToken = null;
    private String TAG ="FacebookFragment";
    public FacebookFragment() {
        // Required empty public constructor
    }

    @Override
    public View onCreateView(LayoutInflater inflater,ViewGroup container,
                             Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.facebook_fragment_main, container, false);
        callbackManager = CallbackManager.Factory.create();
        loginButton = (LoginButton) view.findViewById(R.id.login_button);
        loginButton.setReadPermissions(Arrays.asList("user_posts","user_friends"));
        // If using in a fragment
        loginButton.setFragment(this);
        // Other app specific specialization
// Callback registration
        loginButton.registerCallback(callbackManager, new FacebookCallback<LoginResult>() {
            @Override
            public void onSuccess(LoginResult loginResult) {
                Toast.makeText(getActivity(), "Fb successfully logged in", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onCancel() {
                Toast.makeText(getActivity(),"Fail to login into fb",Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onError(FacebookException exception) {
                exception.printStackTrace();
                Toast.makeText(getActivity(),"Error while login into fb", Toast.LENGTH_SHORT).show();
            }
        });
       /* mFacebookAccessTokenTracker = new AccessTokenTracker() {
            @Override
            protected void onCurrentAccessTokenChanged(AccessToken oldAccessToken, AccessToken currentAccessToken) {
                Toast.makeText(getActivity(), "access token", Toast.LENGTH_SHORT).show();
            }
        };*/
        if(Util.isFbLoggedIn(getActivity().getApplicationContext())==false){

            Toast.makeText(getActivity(),"Fb Logged out.Please login into fb", Toast.LENGTH_SHORT).show();
        }
        return view;
    }
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        callbackManager.onActivityResult(requestCode, resultCode, data);
    }
    public boolean isLoggedIn() {
        AccessToken accessToken = AccessToken.getCurrentAccessToken();
        return accessToken != null;
    }
}
