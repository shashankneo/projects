package com.example.neocfc.activity;

import android.app.Fragment;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.content.Intent;
import android.widget.Toast;

import com.example.neocfc.neored.R;
import com.twitter.sdk.android.Twitter;
import com.twitter.sdk.android.core.Callback;
import com.twitter.sdk.android.core.Result;
import com.twitter.sdk.android.core.TwitterApiClient;
import com.twitter.sdk.android.core.TwitterAuthToken;
import com.twitter.sdk.android.core.TwitterException;
import com.twitter.sdk.android.core.TwitterSession;
import com.twitter.sdk.android.core.identity.TwitterLoginButton;
import com.twitter.sdk.android.core.models.Tweet;
import com.twitter.sdk.android.core.models.TweetEntities;
import com.twitter.sdk.android.core.models.UrlEntity;
import com.twitter.sdk.android.core.models.User;

import java.util.List;

import retrofit.http.GET;
import retrofit.http.Query;
/**
 * Created by neocfc on 6/24/15.
 */
public class TwitterFragment extends Fragment {
    private TwitterLoginButton loginButton;
    private TwitterSession session =null;
    String TAG = "TwitterFragment";
    @Override
    public View onCreateView(LayoutInflater inflater,ViewGroup container,
                             Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.twitter_fragment_main, container, false);
        loginButton = (TwitterLoginButton) view.findViewById(R.id.twitter_login_button);
         session =
                Twitter.getSessionManager().getActiveSession();
        TwitterAuthToken authToken = null;
        if(session!=null)
        {
             authToken = session.getAuthToken();
        }
        if(authToken==null || session ==null)
        {
            Log.v(TAG, " Twitter user not logged in");
        }
       /* else
        {
            String token = authToken.token;
            String secret = authToken.secret;
            Log.v(TAG,"Twitter user logged in token="+token+" secret="+secret);
            getTimeline();
        }*/

        loginButton.setCallback(new Callback<TwitterSession>() {
            @Override
            public void success(Result<TwitterSession> result) {
                // Do something with result, which provides a TwitterSession for making API calls
                Log.v(TAG," Twitter session id="+result.data);
                Toast.makeText(getActivity(), "Twitter successfully logged in", Toast.LENGTH_SHORT).show();
               // getTimeline();
            }

            @Override
            public void failure(TwitterException exception) {
                Log.v(TAG," Twitter login failed="+exception.toString()+"msg="+exception.getMessage());
                Toast.makeText(getActivity(), "Twitter failed to logged in", Toast.LENGTH_SHORT).show();
            }
        });
        return view;
    }
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        loginButton.onActivityResult(requestCode, resultCode, data);
    }

   /* private void getTimeline()
    {
        MyTwitterApiClient twitterApiClient = new MyTwitterApiClient(session);
        CustomService customService = twitterApiClient.getCustomService();
        customService.show( new Callback<List<Tweet>>() {
            @Override
            public void success(Result<List<Tweet>> result) {
                //Do something with result, which provides a Tweet inside of result.data
                List<Tweet> listTweets = result.data;
                for(Tweet obj : listTweets)
                {
                    Log.v(TAG, "tweet="+obj.text);
                    TweetEntities objentity = obj.entities;
                    if(objentity != null)
                    {
                        for(UrlEntity urlObj : objentity.urls)
                        {
                            Log.v(TAG,"Url="+urlObj.url);
                        }
                    }
                    else
                    {
                        Log.v(TAG," No url");
                    }

                }

            }

            public void failure(TwitterException exception) {
                Log.v(TAG, "Timeline not obtained="+exception.toString());
            }
        });
    }
    class MyTwitterApiClient extends TwitterApiClient {
        public MyTwitterApiClient(TwitterSession session) {
            super(session);
        }


        public CustomService getCustomService() {
            return getService(CustomService.class);
        }
    }

    // example users/show service endpoint
    interface CustomService {
        @GET("/1.1/statuses/home_timeline.json")
        void show(Callback<List<Tweet>> cb);
    }*/
}
