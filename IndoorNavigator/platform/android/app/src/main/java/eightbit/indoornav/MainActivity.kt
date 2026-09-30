// ==============================================================================
// Source Origin: Arun (Feature Developer) - Android Target Shell
// Description: MainActivity hosting the OpenGL ES NavGlSurfaceView and interactive
//              guidance HUD controls (floor switcher, destination selector).
//
// Modifications:
//   - [HOW]: Package shortened to eightbit.indoornav. Created standard Activity layout
//     programmatically with overlay controls dispatched via queueEvent() into NativeEngine.
//   - [WHY]: Provides immediate UI interactivity for testing Arun's multi-floor renderer
//     and JNI event dispatching on real Android devices.
// ==============================================================================

package eightbit.indoornav

import android.graphics.Color
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.Gravity
import android.view.ViewGroup
import android.widget.Button
import android.widget.FrameLayout
import android.widget.LinearLayout
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var glView: NavGlSurfaceView
    private lateinit var statusText: TextView
    private val handler = Handler(Looper.getMainLooper())
    private val updateStatusRunnable = object : Runnable {
        override fun run() {
            statusText.text = NativeEngine.nativeGetStatus()
            handler.postDelayed(this, 250) // Refresh status HUD 4 times per second
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val rootLayout = FrameLayout(this).apply {
            layoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        }

        // 1. OpenGL ES View
        glView = NavGlSurfaceView(this)
        rootLayout.addView(glView)

        // 2. HUD Overlay Container
        val overlay = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL
            setPadding(32, 32, 32, 48)
            layoutParams = FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.WRAP_CONTENT,
                Gravity.BOTTOM
            )
        }

        // Status HUD
        statusText = TextView(this).apply {
            setTextColor(Color.WHITE)
            setBackgroundColor(Color.parseColor("#CC111827"))
            setPadding(24, 16, 24, 16)
            textSize = 12f
            typeface = android.graphics.Typeface.MONOSPACE
        }
        overlay.addView(statusText)

        // Floor Switch Row
        val floorRow = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER
            setPadding(0, 16, 0, 8)
        }
        listOf(1, 2, 3).forEach { floor ->
            val btn = Button(this).apply {
                text = "L$floor"
                setOnClickListener {
                    glView.queueEvent { NativeEngine.nativeSetFloor(floor) }
                }
            }
            floorRow.addView(btn)
        }
        overlay.addView(floorRow)

        // Navigation Action Row
        val actionRow = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER
        }
        val routeBtn = Button(this).apply {
            text = "Route 204"
            setOnClickListener {
                glView.queueEvent { NativeEngine.nativeSelectDestination(204, true) }
            }
        }
        val stepBtn = Button(this).apply {
            text = "Step >>"
            setOnClickListener {
                glView.queueEvent { NativeEngine.nativeAdvanceWaypoint() }
            }
        }
        val cancelBtn = Button(this).apply {
            text = "Cancel"
            setOnClickListener {
                glView.queueEvent { NativeEngine.nativeCancelNavigation() }
            }
        }
        actionRow.addView(routeBtn)
        actionRow.addView(stepBtn)
        actionRow.addView(cancelBtn)
        overlay.addView(actionRow)

        rootLayout.addView(overlay)
        setContentView(rootLayout)
    }

    override fun onResume() {
        super.onResume()
        glView.onResume()
        handler.post(updateStatusRunnable)
    }

    override fun onPause() {
        super.onPause()
        glView.onPause()
        handler.removeCallbacks(updateStatusRunnable)
    }
}
