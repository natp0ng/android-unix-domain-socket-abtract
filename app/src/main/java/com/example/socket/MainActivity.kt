package com.example.socket

import android.net.LocalServerSocket
import android.net.LocalSocket
import android.net.LocalSocketAddress
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.ParcelFileDescriptor
import android.util.Log
import android.widget.Toast
import com.example.socket.databinding.ActivityMainBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.io.IOException
import java.net.InetSocketAddress
import java.net.ServerSocket

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val directory = File(filesDir, "mydir")
        if (!directory.exists()) {
            if (!directory.mkdirs()) {
                Log.e(TAG, "Error creating directory")
            }
            Log.d(TAG, "Created directory: ${directory.absolutePath}")
        }

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()

        binding.startAbstractServerButton.setOnClickListener {
            startServerWithAbstractInternal()
        }

        binding.startFilesystemServerButton.setOnClickListener {
            startServerWithFileSystemInternal()
        }

        binding.stopServerButton.setOnClickListener {
            stopServer()
        }

        binding.talkToAbstractServerButton.setOnClickListener {
            talkToAbstractSocketServerInternal()
        }

        binding.talkToFilesystemServerButton.setOnClickListener {
            talkToFileSystemSocketServerInternal()
        }

        binding.startAbstractServerFromKotlinButton.setOnClickListener {
            startAbstractSocketServerFromKotlin()
        }

        binding.talkToAbstractServerFromKotlinButton .setOnClickListener {
            talkToAbstractSocketServerFromKotlin()
        }

        binding.startFilesystemServerFromKotlinButton.setOnClickListener {
            startFileSystemSocketServerFromKotlin()
        }

        binding.talkToFilesystemServerFromKotlinButton .setOnClickListener {
            talkToFileSystemSocketServerFromKotlin()
        }
    }

    /**
     * A native method that is implemented by the 'socket' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    external fun startServer(): Int
    external fun startServerWithAbstract(): Int
    external fun startServerWithFileSystem(): Int
    private external fun stopServer(): Int
    private external fun talkToAbstractSocketServer(): String
    private external fun talkToFileSystemSocketServer(): String

    companion object {
        // Used to load the 'socket' library on application startup.
        init {
            System.loadLibrary("socket")
        }

        private const val SOCKET_ABSTRACT_ADDRESS = "mysocket"
        private const val SOCKET_FILESYSTEM_ADDRESS = "/data/user/0/com.example.socket/files/mydir/mysocket"
        private const val SOCKET_BUFFER_SIZE = 256
        private const val TAG = "MyNativeCode"
        private var serverRunning = false
    }

    private fun startServerWithAbstractInternal() {
        GlobalScope.launch(Dispatchers.IO) {
            // Call the JNI function to start the server on a background thread
            val result = startServerWithAbstract()
        }
    }

    private fun startServerWithFileSystemInternal() {
        GlobalScope.launch(Dispatchers.IO) {
            // Call the JNI function to start the server on a background thread
            val result = startServerWithFileSystem()
        }
    }

    private fun talkToAbstractSocketServerInternal() {
        GlobalScope.launch(Dispatchers.IO) {
            // Call the JNI function to start the server on a background thread
            val result = talkToAbstractSocketServer()
        }
    }

    private fun talkToFileSystemSocketServerInternal() {
        GlobalScope.launch(Dispatchers.IO) {
            // Call the JNI function to start the server on a background thread
            val result = talkToFileSystemSocketServer()
        }
    }

    private fun startAbstractSocketServerFromKotlin() {
        GlobalScope.launch(Dispatchers.IO) {
            serverRunning = true

            val serverSocket = LocalServerSocket(SOCKET_ABSTRACT_ADDRESS)

            // Print the socket address after the socket is bound and listening
            val addressStr = serverSocket.localSocketAddress?.name ?: ""
            Log.i(TAG, "Server started listening on address: $addressStr")

            while (serverRunning) {
                val clientSocket = serverSocket.accept()
                try {
                    Log.i(TAG, "Client connected.")

                    val inputStream = clientSocket.inputStream
                    val buffer = ByteArray(SOCKET_BUFFER_SIZE)
                    val n = inputStream.read(buffer)

                    if (n == -1) {
                        Log.e(TAG, "Error reading message from client")
                        clientSocket.close()
                        continue
                    }

                    val message = String(buffer, 0, n)
                    Log.i(TAG, "Received message from client: $message")

                    val outputStream = clientSocket.outputStream
                    val response = "Hello, client!"
                    outputStream.write(response.toByteArray())
                    outputStream.flush()

                    Log.i(TAG, "Sent message to client: $response")
                } catch (e: IOException) {
                    Log.e(TAG, "Error communicating with client:", e)
                } finally {
                clientSocket.close()
                }
            }

            serverSocket.close()
        }
    }

    private fun talkToAbstractSocketServerFromKotlin(): String {
        val socket = LocalSocket()
        try {
            // Connect to the server using a LocalSocket
            Log.d(TAG, "(kotlin) Connecting to socket...")
            socket.connect(LocalSocketAddress(SOCKET_ABSTRACT_ADDRESS, LocalSocketAddress.Namespace.ABSTRACT))
            Log.d(TAG, "(kotlin) Connected to socket!")
            val os = socket.outputStream
            val `is` = socket.inputStream

            // Send a message to the server
            val message = "Hello from Kotlin!"
            os.write(message.toByteArray(Charsets.UTF_8))
            os.flush()

            Log.d(TAG, "(kotlin) Sent message to server: $message")

            // Receive a response from the server
            val buffer = ByteArray(SOCKET_BUFFER_SIZE)
            val n = `is`.read(buffer)
            if (n == -1) {
                throw IOException("Failed to read data from server")
            }
            val response = String(buffer, 0, n, Charsets.UTF_8)

            Log.d(TAG, "(kotlin) Received message from server: $response")

            // Close the socket and return the response
            socket.close()
            return response
        } catch (e: Exception) {
            Log.e(TAG, "(kotlin) Error talking to server: ${e.message}")
            socket.close()
            return ""
        }
    }

    private fun startFileSystemSocketServerFromKotlin() {
        GlobalScope.launch(Dispatchers.IO) {
            serverRunning = true

        }
    }

    private fun talkToFileSystemSocketServerFromKotlin(): String {
        val socket = LocalSocket()
        try {
            // Connect to the server using a LocalSocket
            Log.d(TAG, "(kotlin) Connecting to socket...")
            socket.connect(LocalSocketAddress(SOCKET_FILESYSTEM_ADDRESS, LocalSocketAddress.Namespace.FILESYSTEM))
            Log.d(TAG, "(kotlin) Connected to socket!")
            val os = socket.outputStream
            val `is` = socket.inputStream

            // Send a message to the server
            val message = "Hello from Kotlin!"
            os.write(message.toByteArray(Charsets.UTF_8))
            os.flush()

            Log.d(TAG, "(kotlin) Sent message to server: $message")

            // Receive a response from the server
            val buffer = ByteArray(SOCKET_BUFFER_SIZE)
            val n = `is`.read(buffer)
            if (n == -1) {
                throw IOException("Failed to read data from server")
            }
            val response = String(buffer, 0, n, Charsets.UTF_8)

            Log.d(TAG, "(kotlin) Received message from server: $response")

            // Close the socket and return the response
            socket.close()
            return response
        } catch (e: Exception) {
            Log.e(TAG, "(kotlin) Error talking to server: ${e.message}")
            socket.close()
            return ""
        }
    }
}