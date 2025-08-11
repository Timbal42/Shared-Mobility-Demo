package com.infineon.blockchain.velink;

import android.annotation.TargetApi;
import android.app.ProgressDialog;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

public class UnlockingActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_unlocking);

        AsyncTaskRunner runner = new AsyncTaskRunner();

        runner.execute();

    }

    private void launchRidingActivity() {
        Intent intent = new Intent(this.getBaseContext(), RidingActivity.class);
        startActivity(intent);
    }

    private void launchDoneActivity() {
        Intent intent = new Intent(this.getBaseContext(), ListActivity.class);
        startActivity(intent);
    }

    private class AsyncTaskRunner extends AsyncTask<String, String, String> {

        private String resp;
        ProgressDialog progressDialog;

        @Override
        protected String doInBackground(String... params) {
            publishProgress("Sleeping..."); // Calls onProgressUpdate()
            try {
                do {
                    Thread.sleep(500, 0);
                } while(Global.btDevice.readLockStatus(getBaseContext()) != 1);
                resp = "success";
            } catch (Exception e) {
                e.printStackTrace();
                resp = e.getMessage();
            }
            return resp;
        }

        @Override
        protected void onPostExecute(String result) {
            if(resp != "success"){
                Log.d("unlocking", "problem");
                //Toast.makeText(getBaseContext(), "Error while unlocking", Toast.LENGTH_LONG).show();

                launchDoneActivity();
            }
            else {
                Log.d("unlocking", "UNLOCKED!!!");
                launchRidingActivity();
            }
        }
    }
}