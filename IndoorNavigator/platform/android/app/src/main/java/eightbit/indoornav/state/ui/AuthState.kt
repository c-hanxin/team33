package eightbit.indoornav.state.ui

import android.util.Log
import eightbit.indoornav.state.AppContext
import eightbit.indoornav.state.AppEvent

class AuthState : IAppState {
    companion object {
        private const val TAG = "AuthState"
    }

    override fun onEnter(context: AppContext) {
        Log.d(TAG, "[Call] AuthState::onEnter")
    }

    override fun onUpdate(context: AppContext, deltaTime: Float) {
        // Handle token expiration or auth polling if needed
    }

    override fun onExit(context: AppContext) {
        Log.d(TAG, "[Call] AuthState::onExit")
    }

    override fun handleEvent(context: AppContext, event: AppEvent) {
        Log.d(TAG, "[Call] AuthState::handleEvent: ${event.type}")
        when (event.type) {
            "AUTH_SUCCESS" -> {
                context.isAuthenticated = true
                if (event.payloadString.isNotEmpty()) {
                    context.userEmail = event.payloadString
                }
            }
            "AUTH_LOGOUT" -> {
                context.isAuthenticated = false
                context.userEmail = ""
            }
        }
    }

    override fun getType(): AppStateType = AppStateType.Auth
}
