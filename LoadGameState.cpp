#include "LoadGameState.hpp"
#include "Messages.hpp"

#include <SFML/Window/Mouse.hpp>
#include <xygine/App.hpp>
#include <xygine/ui/Selection.hpp>
#include <xygine/ui/Label.hpp>
#include <xygine/Resource.hpp>
#include <xygine/FileSystem.hpp>
#include <xygine/ui/Button.hpp>


LoadGameState::LoadGameState(xy::StateStack & stack, xy::State::Context & context)
    : xy::State(stack, context),
    m_UIContainer(context.appInstance.getMessageBus())
{

    //create the greyout the size of the screen
    m_greyout.setSize(context.defaultView.getSize());
    m_greyout.setFillColor({ 128, 128, 128, 100 }); //slightly grey

    //"Select Game:" label
    auto label = xy::UI::create<xy::UI::Label>(m_fonts.get("Westmeath.ttf"));
    label->setString("Select Game:");
    label->setAlignment(xy::UI::Alignment::Centre);
    label->setPosition({ 0,-100 });
    m_UIContainer.addControl(label);

    //game selection box
    m_fileSelection = xy::UI::create<xy::UI::Selection>(m_fonts.get("Westmeath.ttd"), m_textures.get("assets/generic/SelectionArrow.png"));
    m_fileSelection->setAlignment(xy::UI::Alignment::Centre);

    //find all save file names
    auto saveFilePath = "saves/";
    auto files = xy::FileSystem::listFiles(saveFilePath);

    //add each one to the selection
    auto index(0);
    for (auto& file : files)
    {
        m_fileSelection->addItem(file, index++);
    }

    m_UIContainer.addControl(m_fileSelection);

    //load button
    auto button = xy::UI::create<xy::UI::Button>(m_fonts.get("Westmeath.ttd"), m_textures.get());
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setString("Load");
    button->setPosition({ 0,100 });
    button->addCallback([this]()
    {
        //go to playing state
        requestStackPop();
        //load the selected file
        auto msg = getContext().appInstance.getMessageBus().post<std::string>(Messages::LOAD_WORLD);
        *msg = m_fileSelection->getSelectedText();
    });
    m_UIContainer.addControl(button); 

    m_UIContainer.setAlignment(xy::UI::Alignment::Centre);
    m_UIContainer.setPosition(context.defaultView.getCenter());
}


bool LoadGameState::handleEvent(const sf::Event & evt)
{
    sf::Vector2f mPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(getContext().renderWindow));

    m_UIContainer.handleEvent(evt, mPos);
    return false;
}

void LoadGameState::handleMessage(const xy::Message & msg)
{
}

bool LoadGameState::update(float dt)
{
    m_UIContainer.update(dt);
    return false;
}

void LoadGameState::draw()
{
    auto& rt = getContext().renderWindow;
    rt.setView(rt.getDefaultView());
    rt.draw(m_UIContainer);
}