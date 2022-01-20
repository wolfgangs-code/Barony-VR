//! @file Widget.hpp

#pragma once

#include "../main.hpp"

class Frame;

class Widget {
public:
    Widget() = default;
    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    virtual ~Widget();

    Widget& operator=(const Widget&) = delete;
    Widget& operator=(Widget&&) = delete;

    //! widget type
    enum type_t {
        WIDGET_FRAME,
        WIDGET_BUTTON,
        WIDGET_FIELD,
        WIDGET_SLIDER
    };

    virtual type_t  getType() const = 0;
    Widget*         getParent() { return parent; }
    const char*		getName() const { return name.c_str(); }
    bool			isPressed() const { return reallyPressed; }
    bool			isHighlighted() const { return highlighted; }
    bool			isSelected() const { return selected; }
    bool			isDisabled() const { return disabled; }
    bool            isInvisible() const { return invisible; }
    bool            isHideGlyphs() const { return hideGlyphs; }
    bool            isHideKeyboardGlyphs() const { return hideKeyboardGlyphs; }
    bool            isHideSelectors() const { return hideSelectors; }
    Uint32          getHighlightTime() const { return highlightTime; }
    Sint32          getOwner() const { return owner; }
    void			(*getTickCallback() const)(Widget&) { return tickCallback; }
    void			(*getDrawCallback() const)(const Widget&, const SDL_Rect) { return drawCallback; }
    const char*     getWidgetSearchParent() const { return widgetSearchParent.c_str(); }
    auto&           getWidgetActions() const { return widgetActions; }
    auto&           getWidgetMovements() const { return widgetMovements; }
    auto&           getWidgets() const { return widgets; }
    const void*     getUserData() const { return userData; }
    void*           getUserData() { return userData; }
    SDL_Rect        getButtonsOffset() const { return buttonsOffset; }
    SDL_Rect        getSelectorOffset() const { return selectorOffset; }

    void	setName(const char* _name) { name = _name; }
    void	setPressed(bool _pressed) { reallyPressed = pressed = _pressed; }
    void	setDisabled(bool _disabled) { disabled = _disabled; }
    void    setInvisible(bool _invisible) { invisible = _invisible; }
    void    setHideGlyphs(bool _hideGlyphs) { hideGlyphs = _hideGlyphs; }
    void    setHideKeyboardGlyphs(bool _hideGlyphs) { hideKeyboardGlyphs = _hideGlyphs; }
    void    setHideSelectors(bool _hideSelectors) { hideSelectors = _hideSelectors; }
    void    setOwner(Sint32 _owner) { owner = _owner; }
    void	setTickCallback(void (*const fn)(Widget&)) { tickCallback = fn; }
    void	setDrawCallback(void (*const fn)(const Widget&, const SDL_Rect)) { drawCallback = fn; }
    void    setWidgetTab(const char* s) { widgetMovements.emplace("MenuTab", s); }
    void    setWidgetRight(const char* s) { widgetMovements.emplace("MenuRight", s); widgetMovements.emplace("AltMenuRight", s); }
    void    setWidgetDown(const char* s) { widgetMovements.emplace("MenuDown", s); widgetMovements.emplace("AltMenuDown", s); }
    void    setWidgetLeft(const char* s) { widgetMovements.emplace("MenuLeft", s); widgetMovements.emplace("AltMenuLeft", s); }
    void    setWidgetUp(const char* s) { widgetMovements.emplace("MenuUp", s); widgetMovements.emplace("AltMenuUp", s); }
    void    setWidgetPageLeft(const char* s) { widgetActions.emplace("MenuPageLeft", s); }
    void    setWidgetPageRight(const char* s) { widgetActions.emplace("MenuPageRight", s); }
    void    setWidgetBack(const char* s) { widgetActions.emplace("MenuCancel", s); }
    void    setWidgetSearchParent(const char* s) { widgetSearchParent = s; }
    void    addWidgetAction(const char* binding, const char* action) { widgetActions.emplace(binding, action); }
    void    addWidgetMovement(const char* binding, const char* action) { widgetMovements.emplace(binding, action); }
    void    setUserData(void* p) { userData = p; }
    void    setButtonsOffset(SDL_Rect r) { buttonsOffset = r; }
    void    setSelectorOffset(SDL_Rect r) { selectorOffset = r; }

    //! recursively locates the head widget for this widget
    //! @return the head widget, which may be this widget
    Widget* findHead();
    const Widget* findHead() const;

    //! scroll the parent frame (if any) to be within our bounds
    virtual void scrollParent();

    //! activate this widget
    virtual void activate();

    //! select this widget
    virtual void select();

    //! deselect this widget
    virtual void deselect();

    //! update this widget for one tick
    void process();

    //! handle inputs on the widget
    //! @return the next widget to select, or nullptr if no widget was selected
    Widget* handleInput();

    //! return true if this widget is the descendant of another widget
    //! @param widget the widget who is supposedly our ancestor
    //! @return true if it is, otherwise false
    bool isChildOf(const Widget& widget) const;

    //! adopt a new widget as one of our children
    //! @param widget the widget to adopt
    void adoptWidget(Widget& widget);

    enum class SearchType {
        DEPTH_FIRST,
        BREADTH_FIRST
    };

    //! find a widget amongst our children
    //! @param name the name of the widget to find
    //! @param recursive true to search recursively or not
    //! @return the widget found, or nullptr if it was not found
    Widget* findWidget(const char* name, bool recursive, SearchType searchType = SearchType::BREADTH_FIRST);
    const Widget* findWidget(const char* name, bool recursive, SearchType searchType = SearchType::BREADTH_FIRST) const;

    //! build a list of all the selected widgets amongst our children
    //! @param outResult a list containing all the selected widgets
    void findSelectedWidgets(std::vector<Widget*>& outResult);

    //! build a list of all the selected widgets amongst our children (const only)
    //! @param outResult a list containing all the selected widgets
    void findSelectedWidgets(std::vector<const Widget*>& outResult) const;

    //! find the widget selected by the specified owner/player
    //! @param owner the player who owns the widget
    //! @return the selected widget or nullptr if it could not be found
    Widget* findSelectedWidget(int owner);

    //! find search parent
    //! @return the search parent, if any
    Frame* findSearchRoot();
    const Frame* findSearchRoot() const;

protected:
    Widget* parent = nullptr;                                       //!< parent widget
    std::list<Widget*> widgets;                                     //!< widget children
    std::string name;                                               //!< widget name
    bool pressed = false;							                //!< pressed state
    bool reallyPressed = false;						                //!< the "actual" pressed state, pre-mouse process
    bool highlighted = false;                                       //!< if true, this widget has the mouse over it
    bool selected = false;							                //!< if true, this widget has focus
    bool disabled = false;							                //!< if true, the widget is unusable and grayed out
    bool invisible = false;                                         //!< if true, widget is both unusable and invisible
	bool toBeDeleted = false;						                //!< if true, the widget will be removed at the end of its process
    bool hideGlyphs = false;                                        //!< true if you don't want to see controller button glyphs on the widget
    bool hideKeyboardGlyphs = true;                                 //!< true if you don't want to see keyboard glyphs on the widget
    bool hideSelectors = false;                                     //!< true if you don't want to see selectors on the borders of this widget
    Uint32 highlightTime = 0u;						                //!< records the time since the widget was highlighted
    Sint32 owner = 0;                                               //!< which player owns this widget (0 = player 1, 1 = player 2, etc)
    SDL_Rect selectorOffset {0, 0, 0, 0};                           //!< offset for x, y, w, h in the selector box
    SDL_Rect buttonsOffset {0, 0, 0, 0};                            //!< offset for x, y in button prompts
    void (*tickCallback)(Widget&) = nullptr;		                //!< the callback to run each frame for this widget
    void (*drawCallback)(const Widget&, const SDL_Rect) = nullptr;  //!< the callback to run after the widget is drawn
    void* userData = nullptr;                                       //!< user data

    std::unordered_map<std::string, std::string>
        widgetActions;                              //!< widgets to select and activate when input is pressed
    std::unordered_map<std::string, std::string>
        widgetMovements;                            //!< widgets to select when input is pressed
    std::string widgetSearchParent;                 //!< widget to search from for actions and movements

    void drawPost(const SDL_Rect size,
        const std::vector<const Widget*>& selectedWidgets,
        const std::vector<const Widget*>& searchParents) const;
};
