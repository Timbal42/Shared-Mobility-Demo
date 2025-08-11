package com.infineon.blockchain.velink;

import android.annotation.TargetApi;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.os.Build;
import android.util.Log;
import android.widget.Toast;

import java.io.Serializable;
import java.util.UUID;

/**
 * Adopted from https://github.com/tutsplus/Android-BluetoothScannerStarterProject
 */
 
public class DeviceItem implements Serializable {

    private String deviceName;
    private String address;
    private boolean connected;
    public BluetoothDevice btDevice;
    public BluetoothGatt gatt;
    public BluetoothGattCharacteristic characteristic;

    public String getDeviceName() {
        return deviceName;
    }

    public boolean getConnected() {
        return connected;
    }

    public String getAddress() {
        return address;
    }

    public void setDeviceName(String deviceName) {
        this.deviceName = deviceName;
    }

    public DeviceItem(String name, String address, String connected){
        this.deviceName = name;
        this.address = address;
        if (connected == "true") {
            this.connected = true;
        }
        else {
            this.connected = false;
        }
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
    public void connectGatt(){
        gatt = btDevice.connectGatt(AppContext.getAppContext(), false,  mGattCallback);
    }

    public BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);
            Log.d("GATTCallback", "onConnectionStateChange ");

            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Global.btDevice.gatt.discoverServices();
            }
        }

        @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            Log.d("test", "onServicesDiscovered:");

            BluetoothGattService service = Global.btDevice.gatt.getService(UUID.fromString(Global.costumService));
            if (service != null) {
                characteristic = service.getCharacteristic(UUID.fromString(Global.costumCharacteristic));
            }

        }

        @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic,
                                         int status) {
            super.onCharacteristicRead(gatt, characteristic, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.d("Read", "Succesfully read characteristic: " + characteristic.getValue().toString());
            } else {
                Log.d("Read", "Characteristic read not successful");
            }
        }

        @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic,
                                         int status) {
            super.onCharacteristicWrite(gatt, characteristic, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.d("Write", "Succesfully wrote characteristic: " + characteristic.getValue().toString());
            } else {
                Log.d("Write", "Characteristic wrote not successful");
            }
        }

        @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
            Log.i("test", "Notification of time characteristic changed on server.");
            final int charValue = characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_UINT32, 0);
        }
    };

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
    public int writeCharacteristic(String data, Context context) {
        if (characteristic == null) {
           Toast.makeText(context, "Characteristic not found", Toast.LENGTH_LONG).show();
            return -1;
        }

        characteristic.setValue(data);
        boolean ret_write = gatt.writeCharacteristic(characteristic);
        if (ret_write == false){
            Toast.makeText(context, "Characteristic write ERROR", Toast.LENGTH_LONG).show();
            return -1;
        }
        //boolean ret_read = gatt.readCharacteristic(characteristic);

        return 0;
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
    public int readLockStatus(Context context) {
        if (characteristic == null) {
            Toast.makeText(context, "Characteristic not found", Toast.LENGTH_LONG).show();
            return -1;
        }
/*
        Log.d("read", "problem");
        boolean ret_read = gatt.readCharacteristic(characteristic);
        if (ret_read == false){
            ret_read = gatt.readCharacteristic(characteristic);
            Toast.makeText(context, "Characteristic read ERROR", Toast.LENGTH_LONG).show();
            return -1;
        }*/

        boolean ret_read = false;
        do {
            ret_read = gatt.readCharacteristic(characteristic);
            //Toast.makeText(context, "Characteristic read ERROR", Toast.LENGTH_LONG).show();
            //return -1;
        }
        while(ret_read == false);

        byte value[] = characteristic.getValue();

        return value[0];
    }
}
