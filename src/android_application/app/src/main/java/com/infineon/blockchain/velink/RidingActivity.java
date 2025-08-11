package com.infineon.blockchain.velink;

import android.annotation.TargetApi;
import android.content.Intent;
import android.os.Build;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class RidingActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_riding);

        Button button = (Button)findViewById(R.id.buttonLock);
        button.setOnClickListener(new View.OnClickListener() {

            @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
            @Override
            public void onClick(View view) {

                if(Global.btDevice.writeCharacteristic("lock0x98672aB68936A5A3d69C549a61f20AD4c7d40979", getBaseContext()) == 0) {
                    launchLockingActivity();
                }
            }
        });
    }

    private void launchLockingActivity() {
        Intent intent = new Intent(this.getBaseContext(), LockingActivity.class);
        startActivity(intent);
    }
}