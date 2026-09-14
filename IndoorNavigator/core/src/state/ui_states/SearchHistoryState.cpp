#include "state/ui_states/SearchHistoryState.h"

#include <iostream>

#include "state/state_managers/AppContext.h"

void SearchHistoryState::OnEnter(AppContext& context) {
    context.activeTab = "SearchHistory";
    std::cout << "[SearchHistoryState] Search history & recent destinations tab active.\n";
}

void SearchHistoryState::OnUpdate(AppContext& context, float deltaTime) {
    (void)context;
    (void)deltaTime;
}

void SearchHistoryState::OnExit(AppContext& context) {
    (void)context;
    std::cout << "[SearchHistoryState] Exiting search history tab.\n";
}

void SearchHistoryState::HandleEvent(AppContext& context, const AppEvent& event) {
    if (event.type == "QUERY_SEARCH") {
        context.searchQuery = event.payloadString;
        std::cout << "[SearchHistoryState] Searched for: " << context.searchQuery << "\n";
    }
}

AppStateType SearchHistoryState::GetType() const {
    return AppStateType::SearchHistory;
}
