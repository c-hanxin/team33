package eightbit.indoornav.state

/**
 * Tracks application-level context and mobile user session data in Kotlin.
 * Owned by [AppStateManager].
 */
data class AppContext(
    var isAuthenticated: Boolean = false,
    var userEmail: String = "",
    var userDisplayName: String = "",
    var activeTab: String = "MapView",
    var isSearchDrawerOpen: Boolean = false,
    var isObstacleModalOpen: Boolean = false,
    var searchQuery: String = "",
    val searchHistory: MutableList<String> = mutableListOf("Room 201", "Room 204", "Elevator Level 1")
)
