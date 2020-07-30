#pragma once

#include <QtWebEngine>
#include <QtWebEngineWidgets>

#include "webenginepage.hpp"

#include <memory>

class WebEngineView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebEngineView(QWidget *parent = nullptr);

    void setPage(WebEnginePage *page);
    WebEnginePage *page() const;

protected:
    WebEngineView *createWindow(QWebEnginePage::WebWindowType type);

private:
    std::unique_ptr<WebEnginePage> _page;
};
