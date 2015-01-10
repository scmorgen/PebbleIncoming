package hackathon.ucsc.spaceteam.com.pebbleincoming;

import android.content.Context;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.widget.TextView;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.PebbleKit.PebbleDataReceiver;
import com.getpebble.android.kit.util.PebbleDictionary;

import java.util.UUID;


public class MainActivity extends ActionBarActivity {
    private TextView mButtonView;
    private static final int
            KEY_BUTTON_EVENT = 0,
            BUTTON_EVENT_UP = 1,
            BUTTON_EVENT_DOWN = 2,
            BUTTON_EVENT_SELECT = 3;

    private PebbleDataReceiver mReceiver;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mButtonView = new TextView(this);
        mButtonView.setText("No button yet!");

        setContentView(mButtonView);
    }

    @Override
    protected void onResume() {
        super.onResume();

        mReceiver = new PebbleDataReceiver(UUID.fromString("7c02f3fb-ff81-4893-aa1c-f741b2e7c3ff")) {

            @Override
            public void receiveData(Context context, int transactionId, PebbleDictionary data) {
                //ACK the message
                PebbleKit.sendAckToPebble(context, transactionId);

                //Check the key exists
                if(data.getUnsignedIntegerAsLong( KEY_BUTTON_EVENT) != null) {
                    int button = data.getUnsignedIntegerAsLong(KEY_BUTTON_EVENT).intValue();

                    switch(button) {
                        case BUTTON_EVENT_UP:
                            mButtonView.setText("UP button pressed!");
                            break;
                        case BUTTON_EVENT_DOWN:
                            mButtonView.setText("DOWN button pressed!");
                            break;
                        case BUTTON_EVENT_SELECT:
                            mButtonView.setText("SELECT button pressed!");
                            break;
                    }
                }
            }

        };

        PebbleKit.registerReceivedDataHandler(this, mReceiver);
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver( mReceiver);
    }
}
