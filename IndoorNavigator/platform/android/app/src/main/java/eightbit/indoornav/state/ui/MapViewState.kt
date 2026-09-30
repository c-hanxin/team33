package eightbit.indoornav.state.ui

import android.util.Log
import eightbit.indoornav.state.AppContext
import eightbit.indoornav.state.AppEvent

class MapViewState : IAppState {
    companion object {
        private const val TAG = "MapViewState"
    }

    override fun onEnter(context: AppContext) {
        Log.d(TAG, "[Call] MapViewState::onEnter")
        context.activeTab = "MapView"
    }

    override fun onUpdate(context: AppContext, deltaTime: Float) {
        // Map view frame tick logic
    }

    override fun onExit(context: AppContext) {
        Log.d(TAG, "[Call] MapViewState::onExit")
    }

    override fun handleEvent(context: AppContext, event: AppEvent) {
        Log.d(TAG, "[Call] MapViewState::handleEvent: ${event.type}")
        when (event.type) {
            "OPEN_SEARCH" -> {
                context.isSearchDrawerOpen = true
            }
            "CLOSE_SEARCH" -> {
                context.isSearchDrawerOpen = false
            }
            "TOGGLE_OBSTACLE_MODAL" -> {
                context.isObstacleModalOpen = !context.isObstacleModalOpen
            }
        }
    }

    override fun getType(): AppStateType = AppStateType.MapView
}
