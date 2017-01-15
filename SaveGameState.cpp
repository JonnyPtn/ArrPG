#include "SaveGameState.hpp"
#include "Messages.hpp"

#include <SFML/Window/Mouse.hpp>
#include <xygine/App.hpp>
#include <xygine/ui/TextBox.hpp>
#include <xygine/ui/Selection.hpp>
#include <xygine/ui/Label.hpp>
#include <xygine/Resource.hpp>
#include <xygine/FileSystem.hpp>
#include <xygine/ui/Button.hpp>


SaveGameState::SaveGameState(xy::StateStack & stack, xy::State::Context & context)
    : xy::State(stack, context),
    m_UIContainer(context.appInstance.getMessageBus())
{
    //game selection box
    m_currentSaveSelection = xy::UI::create<xy::UI::Selection>(m_fonts.get("Westmeath.ttf"), m_textures.get("assets/generic/SelectionArrow.png"));
    
    //find all save file names
    auto saveFilePath = "saves/";
    auto files = xy::FileSystem::listFiles(saveFilePath);

    //add each one to the selection
    auto index(0);
    for (auto& file : files)
    {
        m_currentSaveSelection->addItem(file, index++);
    }
    m_currentSaveSelection->setAlignment(xy::UI::Alignment::Centre);

    //only add this if there's files
    if(files.size())
        m_UIContainer.addControl(m_currentSaveSelection);

    //new save
    m_inputBox = xy::UI::create<xy::UI::TextBox>(m_fonts.get("Westemeath.ttf"));
    m_inputBox->setLabelText("New save:");
    m_inputBox->setAlignment(xy::UI::Alignment::Centre);
    m_UIContainer.addControl(m_inputBox);

    //save button
    auto saveButton = xy::UI::create<xy::UI::Button>(m_fonts.get("Westemeath.ttf"),m_textures.get());
    saveButton->setAlignment(xy::UI::Alignment::Centre);
    saveButton->addCallback([this]()
    {
        //check the input box first
        auto name = m_inputBox->getText();

        //if that's empty, use the selected one
        if (name.empty())
            name = m_currentSaveSelection->getSelectedText();

        //and broadcast the message
        auto msg = getContext().appInstance.getMessageBus().post<std::string>(Messages::SAVE_WORLD);
        *msg = name;
    });
    m_UIContainer.addControl(saveButton);

    m_UIContainer.setAlignment(xy::UI::Alignment::Centre);
    m_UIContainer.setPosition(context.defaultView.getCenter());
}


bool SaveGameState::handleEvent(const sf::Event & evt)
{
    sf::Vector2f mPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(getContext().renderWindow));

    m_UIContainer.handleEvent(evt, mPos);
    return false;
}

void SaveGameState::handleMessage(const xy::Message & msg)
{
}

bool SaveGameState::update(float dt)
{
    m_UIContainer.update(dt);
    return false;
}

void SaveGameState::draw()
{
    auto& rt = getContext().renderWindow;
    rt.setView(rt.getDefaultView());
    rt.draw(m_UIContainer);
}