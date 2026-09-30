package eightbit.indoornav.state

/**
 * Event message dispatched across the Kotlin state system.
 */
data class AppEvent(
    val type: String,
    val payloadString: String = "",
    val payloadInt: Int = 0,
    val payloadBool: Boolean = false
)
