package com.infineon.blockchain.velink;

import android.annotation.TargetApi;
import android.content.Intent;
import android.os.Build;
import android.support.constraint.ConstraintLayout;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class UnlockActivity extends AppCompatActivity {

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_unlock);

        ConstraintLayout layout = (ConstraintLayout)findViewById(R.id.constraintLayout);
        if(Global.btDevice.getDeviceName().contains("Car")) {
            layout.setBackgroundResource(R.drawable.car_unlock_screen);
        }
        else{
            layout.setBackgroundResource(R.drawable.scooter_unlock_screen);
        }

        Button button = (Button)findViewById(R.id.buttonUnlock);
        button.setOnClickListener(new View.OnClickListener() {

            @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
            @Override
            public void onClick(View view) {

                if(Global.btDevice.writeCharacteristic("unlock0x98672aB68936A5A3d69C549a61f20AD4c7d40979", getBaseContext()) == 0){
                    launchUnlockingActivity();
                }
            }
        });
    }

    private void launchUnlockingActivity() {
        Intent intent = new Intent(this.getBaseContext(), UnlockingActivity.class);
        startActivity(intent);
    }
}