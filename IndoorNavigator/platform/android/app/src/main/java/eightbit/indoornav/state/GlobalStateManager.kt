package eightbit.indoornav.state

import android.util.Log
import eightbit.indoornav.NativeEngine
import eightbit.indoornav.state.ui.AppStateType

/**
 * Top-Level Application Orchestrator in Kotlin.
 *
 * Coordinates:
 *   1. [AppStateManager] - Kotlin Tier 1 UI FSM (Authentication, Search, Screens)
 *   2. [NativeEngine]    - C++ Tier 2 Core Simulation Engine (A* Pathfinding, Navigation, GL ES 3.0 Rendering)
 */
class GlobalStateManager private constructor() {
    companion object {
        private const val TAG = "GlobalStateManager"

        @Volatile
        private var instance: GlobalStateManager? = null

        fun getInstance(): GlobalStateManager {
            return instance ?: synchronized(this) {
                instance ?: GlobalStateManager().also { instance = it }
            }
        }
    }

    val appStateManager = AppStateManager()

    /**
     * Central event dispatch hub. Routes events appropriately between
     * Kotlin UI state and C++ native core simulation.
     */
    fun sendEvent(event: AppEvent) {
        Log.d(TAG, "[Call] GlobalStateManager::sendEvent(${event.type})")

        // 1. Dispatch into Kotlin UI FSM
        appStateManager.sendEvent(event)

        // 2. Handle automatic state reactions
        when (event.type) {
            "AUTH_SUCCESS" -> {
                // Once authenticated, transition UI screen into MapView
                appStateManager.changeState(AppStateType.MapView)
            }
            "SWITCH_TAB" -> {
                when (event.payloadString) {
                    "MapView" -> appStateManager.changeState(AppStateType.MapView)
                    "SearchHistory" -> appStateManager.changeState(AppStateType.SearchHistory)
                    "UserProfile" -> appStateManager.changeState(AppStateType.UserProfile)
                }
            }
            "FLOOR_SWITCH" -> {
                NativeEngine.nativeSetFloor(event.payloadInt)
            }
            "DESTINATION_SELECTED" -> {
                NativeEngine.nativeSelectDestination(event.payloadInt, event.payloadBool)
            }
            "WAYPOINT_REACHED" -> {
                NativeEngine.nativeAdvanceWaypoint()
            }
            "CANCEL_NAVIGATION" -> {
                NativeEngine.nativeCancelNavigation()
            }
        }
    }

    /**
     * Returns a combined diagnostic status string joining Kotlin Tier 1 UI state
     * with C++ Tier 2 core simulation state.
     */
    fun getFullStatus(): String {
        val uiState = appStateManager.currentState.getType().name
        val user = if (appStateManager.context.isAuthenticated) appStateManager.context.userEmail else "Guest (Unauthenticated)"
        val coreStatus = NativeEngine.nativeGetStatus()

        return buildString {
            append("[Kotlin Tier 1 UI]: State = $uiState | User = $user\n")
            append(coreStatus)
        }
    }
}
