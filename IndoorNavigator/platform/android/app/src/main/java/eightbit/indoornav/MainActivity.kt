// ==============================================================================
// Source Origin: Arun (Feature Developer) - Android Target Shell
// Description: MainActivity hosting the OpenGL ES NavGlSurfaceView and interactive
//              guidance HUD controls (floor switcher, destination selector).
//
// Modifications:
//   - [HOW]: Package shortened to eightbit.indoornav. Integrated GlobalStateManager
//     to coordinate Kotlin Tier 1 UI FSM (Auth/MapView/Tabs) with C++ Tier 2 Core Engine.
//     All GL actions are queued via glView.queueEvent() to satisfy Arun's threading rule.
//   - [WHY]: Stage 2 Kotlin migration: Kotlin now orchestrates app lifecycle and UI screens,
//     while C++ executes the 3D graphics and pathfinding engine.
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
import eightbit.indoornav.state.AppEvent
import eightbit.indoornav.state.GlobalStateManager

class MainActivity : AppCompatActivity() {

    private lateinit var glView: NavGlSurfaceView
    private lateinit var statusText: TextView
    private val globalStateManager = GlobalStateManager.getInstance()
    private val handler = Handler(Looper.getMainLooper())
    private val updateStatusRunnable = object : Runnable {
        override fun run() {
            statusText.text = globalStateManager.getFullStatus()
            handler.postDelayed(this, 250) // Refresh status HUD 4 times per second
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // Remove default opaque window background to prevent occluding the SurfaceView
        window.setBackgroundDrawable(null)

        val rootLayout = FrameLayout(this).apply {
            layoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        }

        // 1. OpenGL ES View
        glView = NavGlSurfaceView(this).apply {
            layoutParams = FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT
            )
        }
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

        // Combined Status HUD (Kotlin Tier 1 UI + C++ Tier 2 Core Engine)
        statusText = TextView(this).apply {
            setTextColor(Color.WHITE)
            setBackgroundColor(Color.parseColor("#E6111827"))
            setPadding(24, 16, 24, 16)
            textSize = 11f
            typeface = android.graphics.Typeface.MONOSPACE
        }
        overlay.addView(statusText)

        // UI Auth / Tab Row
        val uiRow = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER
            setPadding(0, 12, 0, 4)
        }
        val loginBtn = Button(this).apply {
            text = "Login"
            setOnClickListener {
                globalStateManager.sendEvent(
                    AppEvent(type = "AUTH_SUCCESS", payloadString = "student@sit.singaporetech.edu.sg")
                )
            }
        }
        val profileBtn = Button(this).apply {
            text = "Profile"
            setOnClickListener {
                globalStateManager.sendEvent(AppEvent(type = "SWITCH_TAB", payloadString = "UserProfile"))
            }
        }
        val mapBtn = Button(this).apply {
            text = "Map"
            setOnClickListener {
                globalStateManager.sendEvent(AppEvent(type = "SWITCH_TAB", payloadString = "MapView"))
            }
        }
        uiRow.addView(loginBtn)
        uiRow.addView(profileBtn)
        uiRow.addView(mapBtn)
        overlay.addView(uiRow)

        // Floor Switch Row
        val floorRow = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER
            setPadding(0, 4, 0, 4)
        }
        listOf(1, 2, 3).forEach { floor ->
            val btn = Button(this).apply {
                text = "L$floor"
                setOnClickListener {
                    glView.queueEvent {
                        globalStateManager.sendEvent(AppEvent(type = "FLOOR_SWITCH", payloadInt = floor))
                    }
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
                glView.queueEvent {
                    globalStateManager.sendEvent(
                        AppEvent(type = "DESTINATION_SELECTED", payloadInt = 204, payloadBool = true)
                    )
                }
            }
        }
        val stepBtn = Button(this).apply {
            text = "Step >>"
            setOnClickListener {
                glView.queueEvent {
                    globalStateManager.sendEvent(AppEvent(type = "WAYPOINT_REACHED"))
                }
            }
        }
        val cancelBtn = Button(this).apply {
            text = "Cancel"
            setOnClickListener {
                glView.queueEvent {
                    globalStateManager.sendEvent(AppEvent(type = "CANCEL_NAVIGATION"))
                }
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
