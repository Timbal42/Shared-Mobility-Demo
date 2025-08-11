package com.infineon.blockchain.velink;

import android.annotation.TargetApi;
import android.content.Intent;
import android.os.Build;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class LockedActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_locked);
        Button button = (Button)findViewById(R.id.buttonDone);
        button.setOnClickListener(new View.OnClickListener() {

            @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
            @Override
            public void onClick(View view) {
                Global.btDevice.gatt.disconnect();
                Global.btDevice.gatt.close();
                launchDoneActivity();
            }

        });
    }

    private void launchDoneActivity() {
        Intent intent = new Intent(this.getBaseContext(), ListActivity.class);
        startActivity(intent);
    }

}