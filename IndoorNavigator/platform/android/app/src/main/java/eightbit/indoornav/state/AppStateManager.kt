package eightbit.indoornav.state

import android.util.Log
import eightbit.indoornav.state.ui.*

/**
 * Tier 1 UI Finite State Machine in Kotlin.
 * Manages screen lifecycles, user authentication, and mobile UI state.
 */
class AppStateManager {
    companion object {
        private const val TAG = "AppStateManager"
    }

    val context = AppContext()
    var currentState: IAppState = AuthState()
        private set

    init {
        currentState.onEnter(context)
    }

    fun changeState(newType: AppStateType) {
        if (currentState.getType() == newType) return

        Log.d(TAG, "[Call] AppStateManager::changeState(${newType.name})")
        currentState.onExit(context)
        currentState = createState(newType)
        currentState.onEnter(context)
    }

    fun update(deltaTime: Float) {
        currentState.onUpdate(context, deltaTime)
    }

    fun sendEvent(event: AppEvent) {
        Log.d(TAG, "[Call] AppStateManager::sendEvent(${event.type})")
        currentState.handleEvent(context, event)
    }

    private fun createState(type: AppStateType): IAppState {
        return when (type) {
            AppStateType.Auth -> AuthState()
            AppStateType.MapView -> MapViewState()
            AppStateType.SearchHistory -> SearchHistoryState()
            AppStateType.UserProfile -> UserProfileState()
        }
    }
}
