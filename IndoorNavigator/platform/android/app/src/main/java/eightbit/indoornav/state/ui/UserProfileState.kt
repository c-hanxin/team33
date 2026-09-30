package eightbit.indoornav.state.ui

import android.util.Log
import eightbit.indoornav.state.AppContext
import eightbit.indoornav.state.AppEvent

class UserProfileState : IAppState {
    companion object {
        private const val TAG = "UserProfileState"
    }

    override fun onEnter(context: AppContext) {
        Log.d(TAG, "[Call] UserProfileState::onEnter")
        context.activeTab = "UserProfile"
    }

    override fun onUpdate(context: AppContext, deltaTime: Float) {}

    override fun onExit(context: AppContext) {
        Log.d(TAG, "[Call] UserProfileState::onExit")
    }

    override fun handleEvent(context: AppContext, event: AppEvent) {
        Log.d(TAG, "[Call] UserProfileState::handleEvent: ${event.type}")
        when (event.type) {
            "UPDATE_DISPLAY_NAME" -> {
                if (event.payloadString.isNotBlank()) {
                    context.userDisplayName = event.payloadString
                }
            }
        }
    }

    override fun getType(): AppStateType = AppStateType.UserProfile
}
