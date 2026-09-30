package eightbit.indoornav.state.ui

import eightbit.indoornav.state.AppContext
import eightbit.indoornav.state.AppEvent

enum class AppStateType {
    Auth,
    MapView,
    UserProfile,
    SearchHistory
}

interface IAppState {
    fun onEnter(context: AppContext)
    fun onUpdate(context: AppContext, deltaTime: Float)
    fun onExit(context: AppContext)
    fun handleEvent(context: AppContext, event: AppEvent)
    fun getType(): AppStateType
}
