// ==============================================================================
// Source Origin: Arun (Feature Developer) - Android Target Shell
// Description: GLSurfaceView implementing OpenGL ES 3.0 rendering, gesture handling,
//              and thread-safe event queueing.
//
// Modifications:
//   - [HOW]: Package shortened to eightbit.indoornav. Implemented GLSurfaceView with
//     ScaleGestureDetector and drag gestures. Strictly follows Arun's threading rule:
//     all touch/drag events and lifecycle callbacks are dispatched through queueEvent()
//     so the C++ engine runs lock-free.
//   - [WHY]: Satisfies CSD2401 M1 Requirement #1 and #4 under eightbit.indoornav namespace.
// ==============================================================================

package eightbit.indoornav

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class NavGlSurfaceView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null
) : GLSurfaceView(context, attrs), GLSurfaceView.Renderer {

    private val density = context.resources.displayMetrics.density
    private var lastTouchX = 0f
    private var lastTouchY = 0f
    private var isDragging = false
    private var lastFrameTimeNs = System.nanoTime()

    private val scaleDetector = ScaleGestureDetector(context, object : ScaleGestureDetector.SimpleOnScaleGestureListener() {
        override fun onScale(detector: ScaleGestureDetector): Boolean {
            val factor = detector.scaleFactor
            queueEvent {
                NativeEngine.nativeOnZoom(factor)
            }
            return true
        }
    })

    init {
        setEGLContextClientVersion(3)
        preserveEGLContextOnPause = true
        setRenderer(this)
        renderMode = RENDERMODE_CONTINUOUSLY
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        NativeEngine.nativeOnSurfaceCreated()
        if (width > 0 && height > 0) {
            NativeEngine.nativeOnSurfaceChanged(width, height, density)
        }
        lastFrameTimeNs = System.nanoTime()
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        NativeEngine.nativeOnSurfaceChanged(width, height, density)
    }

    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        super.onSizeChanged(w, h, oldw, oldh)
        if (w > 0 && h > 0) {
            queueEvent {
                NativeEngine.nativeOnSurfaceChanged(w, h, density)
            }
        }
    }

    override fun onDrawFrame(gl: GL10?) {
        val now = System.nanoTime()
        val dt = ((now - lastFrameTimeNs) / 1_000_000_000.0f).coerceIn(0.001f, 0.1f)
        lastFrameTimeNs = now
        NativeEngine.nativeOnDrawFrame(dt)
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        scaleDetector.onTouchEvent(event)
        if (scaleDetector.isInProgress) {
            isDragging = false
            return true
        }

        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                lastTouchX = event.x
                lastTouchY = event.y
                isDragging = true
            }
            MotionEvent.ACTION_MOVE -> {
                if (isDragging) {
                    val dx = event.x - lastTouchX
                    val dy = event.y - lastTouchY
                    lastTouchX = event.x
                    lastTouchY = event.y
                    queueEvent {
                        NativeEngine.nativeOnDrag(dx, dy)
                    }
                }
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                isDragging = false
            }
        }
        return true
    }
}
