package com.example.socket

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import com.example.socket.databinding.ActivityMainBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()

        // Set click listeners for the Button views
        binding.startServerButton.setOnClickListener {
            Log.d("iqteam","start server......")
            startServerInternal()
        }

        binding.stopServerButton.setOnClickListener {
            Log.d("iqteam", "stop server......")
            stopServer()
        }

        binding.talkToServerButton.setOnClickListener {
            Log.d("iqteam", "talk to server......")
            talkToServerInternal()
        }
    }

    /**
     * A native method that is implemented by the 'socket' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    external fun startServer(): Int
    external fun startServer1(): Int
    private external fun stopServer(): Int
    private external fun talkToServer(): String

    companion object {
        // Used to load the 'socket' library on application startup.
        init {
            System.loadLibrary("socket")
        }
    }

    private fun startServerInternal() {
        GlobalScope.launch(Dispatchers.IO) {
            // Call the JNI function to start the server on a background thread
            val result = startServer1()

            // Switch back to the main thread to display a Toast message
//            launch(Dispatchers.Main) {
//                if (result == 0) {
//                    Toast.makeText(this@MainActivity, "Server started", Toast.LENGTH_SHORT).show()
//                } else {
//                    Toast.makeText(this@MainActivity, "Failed to start server", Toast.LENGTH_SHORT).show()
//                }
//            }
        }
    }

    private fun talkToServerInternal() {
        GlobalScope.launch(Dispatchers.IO) {
            // Call the JNI function to start the server on a background thread
            val result = talkToServer()

            // Switch back to the main thread to display a Toast message
//            launch(Dispatchers.Main) {
//                if (result == 0) {
//                    Toast.makeText(this@MainActivity, "Server started", Toast.LENGTH_SHORT).show()
//                } else {
//                    Toast.makeText(this@MainActivity, "Failed to start server", Toast.LENGTH_SHORT).show()
//                }
//            }
        }
    }
}