package eightbit.indoornav.state.ui

import android.util.Log
import eightbit.indoornav.state.AppContext
import eightbit.indoornav.state.AppEvent

class SearchHistoryState : IAppState {
    companion object {
        private const val TAG = "SearchHistoryState"
    }

    override fun onEnter(context: AppContext) {
        Log.d(TAG, "[Call] SearchHistoryState::onEnter")
        context.activeTab = "SearchHistory"
    }

    override fun onUpdate(context: AppContext, deltaTime: Float) {}

    override fun onExit(context: AppContext) {
        Log.d(TAG, "[Call] SearchHistoryState::onExit")
    }

    override fun handleEvent(context: AppContext, event: AppEvent) {
        Log.d(TAG, "[Call] SearchHistoryState::handleEvent: ${event.type}")
        when (event.type) {
            "ADD_SEARCH_ENTRY" -> {
                if (event.payloadString.isNotBlank()) {
                    context.searchHistory.add(0, event.payloadString)
                }
            }
            "CLEAR_SEARCH_HISTORY" -> {
                context.searchHistory.clear()
            }
        }
    }

    override fun getType(): AppStateType = AppStateType.SearchHistory
}
