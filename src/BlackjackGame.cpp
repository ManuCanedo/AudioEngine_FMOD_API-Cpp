#include <fmod.hpp>
#include "AudioManager.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <ctime>
#include <algorithm>
#include <thread>

using namespace std;

/*
*               Hand                               Card          Game
*     ___________|_____________
*    |                         |
*   Deck                 GenericPlayer
*                 _____________|_____________
*                |                           |
*              Player                      House
*/

// CLASS DECLARATIONS
//////////
class Card
{
public:
    enum rank {
        ACE = 1, TWO, THREE, FOUR, FIVE, SIX, SEVE, EIGHT, NINE, TEN,
        JACK, QUEEN, KING
    };
    enum suit { CLUBS, DIAMONDS, HEARTS, SPADES };

    friend ostream& operator<< (ostream& os, const Card& aCard);

    Card(rank r = ACE, suit s = SPADES, bool ifu = true);

    int GetValue() const;
    void Flip();

private:
    rank m_Rank;
    suit m_Suit;
    bool m_IsFaceUp;
};

class Hand
{
public:
    Hand();
    virtual ~Hand();

    void Add(Card* pCard);
    void Clear();
    int GetTotal() const;

protected:
    vector<Card*> m_Cards;
};

class GenericPlayer : public Hand
{
    friend ostream& operator<< (ostream& os, const GenericPlayer& aGP);

public:
    GenericPlayer(const string& name = "");

    virtual ~GenericPlayer();

    FMOD_VECTOR pos;
    FMOD_VECTOR vel;

    virtual bool IsHitting() const = 0; // pure virtual
    bool IsBusted() const;
    void Bust() const;

protected:
    string m_Name;
};

class Player : public GenericPlayer
{
public:
    Player(const string& name = "", int playerNo = 0, int totalPlayers = 1);

    virtual ~Player(); // I use virtual keywords as reminders
    virtual bool IsHitting() const; // This functions have to be overriden

    void Win() const;
    void Lose() const;
    void Push() const;
    FMOD_VECTOR getPos();
    FMOD_VECTOR getVel();

};

class House : public GenericPlayer
{
public:
    House(const string& name = "House");
    virtual ~House();

    virtual bool IsHitting() const;
    void FlipFirstCard();
};

class Deck : public Hand
{
public:
    Deck();
    virtual ~Deck();

    void Populate();
    void Shuffle();
    void Deal(Hand& aHand);
    void AdditionalCards(GenericPlayer& aGenericPlayer, AudioManager& audio);
};

class Game
{
public:
    Game(const vector<string>& names);

    ~Game();

    //plays the game of blackjack    
    void Play(AudioManager& audio);

private:
    Deck m_Deck;
    House m_House;
    vector<Player> m_Players;
};

// MEMBER DEFINITIONS
//////////
Card::Card(rank r, suit s, bool ifu) : m_Rank(r), m_Suit(s), m_IsFaceUp(ifu)
{}

int Card::GetValue() const
{
    int value = 0;
    if (m_IsFaceUp)
    {
        value = m_Rank;
        if (value > 10)
        {
            value = 10;
        }
    }
    return value;
}

void Card::Flip()
{
    m_IsFaceUp = !(m_IsFaceUp);
}

Hand::Hand()
{
    m_Cards.reserve(7);
}

Hand::~Hand()
{
    Clear();
}

void Hand::Add(Card* pCard)
{
    m_Cards.push_back(pCard);
}

void Hand::Clear()
{
    vector<Card*>::iterator iter = m_Cards.begin();
    for (iter = m_Cards.begin(); iter != m_Cards.end(); iter++)
    {
        delete* iter;
        *iter = 0;
    }
    m_Cards.clear();
}

int Hand::GetTotal() const
{
    //if no cards in hand, return 0
    if (m_Cards.empty())
    {
        return 0;
    }

    //if a first card has value of 0, then card is face down; return 0
    if (m_Cards[0]->GetValue() == 0)
    {
        return 0;
    }

    //add up card values, treat each Ace as 1
    int total = 0;
    vector<Card*>::const_iterator iter;
    for (iter = m_Cards.begin(); iter != m_Cards.end(); ++iter)
    {
        total += (*iter)->GetValue();
    }

    //determine if hand contains an Ace
    bool containsAce = false;
    for (iter = m_Cards.begin(); iter != m_Cards.end(); ++iter)
    {
        if ((*iter)->GetValue() == Card::ACE)
        {
            containsAce = true;
        }
    }

    //if hand contains Ace and total is low enough, treat Ace as 11
    if (containsAce && total <= 11)
    {
        //add only 10 since we've already added 1 for the Ace
        total += 10;
    }

    return total;
}

GenericPlayer::GenericPlayer(const string& name) : m_Name(name)
{}

GenericPlayer::~GenericPlayer()
{}

bool GenericPlayer::IsBusted() const
{
    return (GetTotal() > 21);
}

void GenericPlayer::Bust() const
{
    cout << m_Name << " busts.\n";
}

Player::Player(const string& name, int playerNo, int totalPlayers) : GenericPlayer(name)
{
    switch (playerNo)
    {
    case 0:
        this->pos = { 0.4f, -0.5f, 0.0f };
        this->vel = { 0.0f, 0.0f, 0.0f };
        break;
    case 1:
        if (totalPlayers < 4)
        {
            this->pos = { 0.7f, -0.5f, 1.2f };
            this->vel = { 0.0f, 0.0f, 0.0f };
            break;
        }
        else if (totalPlayers < 6) 
        {
            this->pos = { 0.5f, -0.5f, 0.7f };
            this->vel = { 0.0f, 0.0f, 0.0f };
            break;
        }
        else
        {
            this->pos = { 0.5f, -0.5f, 0.7f };
            this->vel = { 0.2f, -0.5f, 0.0f };
            break;
        }
    case 2:
        if (totalPlayers < 4)
        {
            this->pos = { 0.7f, -0.5f, -1.2f };
            this->vel = { 0.0f, 0.0f, 0.0f };
            break;
        }
        else if (totalPlayers < 6)
        {
            this->pos = { 0.7f, -0.5f, 1.2f };
            this->vel = { 0.0f, 0.0f, 0.0f };
            break;
        }
        else
        {
            this->pos = { 0.6f, -0.5f, 1.0f };
            this->vel = { 0.0f, 0.0f, 0.0f };
            break;
        }
    case 3:
        if (totalPlayers < 6)
        {
            this->pos = { 0.7f, -0.5f, -1.2f };
            this->vel = { 0.0f, 0.0f, 0.0f };
            break;
        }
        else
        {
            this->pos = { 0.7f, -0.5f, 1.2f };
            this->vel = { 0.0f, 0.0f, 0.0f };
            break;
        }
    case 4:
        if (totalPlayers < 6)
        {
            this->pos = { 0.5f, -0.5f, -0.7f };
            this->vel = { 0.0f, 0.0f, 0.0f };
            break;
        }
        else
        {
            this->pos = { 0.7f, -0.5f, -1.2f };
            this->vel = { 0.0f, 0.0f, 0.0f };
            break;
        }
    case 5:
        this->pos = { 0.6f, -0.5f, -1.0f };
        this->vel = { 0.0f, 0.0f, 0.0f };
        break;
    case 6:
        this->pos = { 0.5f, -0.5f, -0.7f };
        this->vel = { 0.0f, 0.0f, 0.0f };
        break;
    default:
        cout << "SOME ERROR CREATING PLAYERS\n";
        break;
    }
}

Player::~Player()
{}

bool Player::IsHitting() const
{
    cout << m_Name << ", do you want a hit? (Y/N): ";
    char response;
    cin >> response;
    return (response == 'y' || response == 'Y');
}

void Player::Win() const
{
    cout << m_Name << " wins.\n";
}

void Player::Lose() const
{
    cout << m_Name << " loses.\n";
}

void Player::Push() const
{
    cout << m_Name << " pushes.\n";
}

FMOD_VECTOR Player::getPos()
{
    return this->pos;
}

FMOD_VECTOR Player::getVel()
{
    return this->pos;
}

House::House(const string& name) : GenericPlayer(name)
{}

House::~House()
{}

bool House::IsHitting() const
{
    return (GetTotal() <= 16);
}

void House::FlipFirstCard()
{
    if (!(m_Cards.empty()))
    {
        m_Cards[0]->Flip();
    }
    else
    {
        cout << "No card to flip!\n";
    }
}

Deck::Deck()
{
    m_Cards.reserve(52);
    Populate();
}

Deck::~Deck()
{}

void Deck::Populate()
{
    Clear();

    for (int s = Card::CLUBS; s <= Card::SPADES; ++s)
    {
        for (int r = Card::ACE; r <= Card::KING; ++r)
        {
            Add(new Card(static_cast<Card::rank>(r), static_cast<Card::suit>(s)));
        }
    }
}

void Deck::Shuffle()
{
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(m_Cards.begin(), m_Cards.end(), default_random_engine(seed));
}

void Deck::Deal(Hand& aHand)
{
    if (!(m_Cards.empty()))
    {
        aHand.Add(m_Cards.back());
        m_Cards.pop_back();
    }
    else
    {
        cout << "Out of cards on current deck. I using the next one.\n";
        Populate();
    }
}

void Deck::AdditionalCards(GenericPlayer& aGenericPlayer, AudioManager& audio)
{
    cout << endl;
    //continue to deal a card as long as generic player isn't busted and
    //wants another hit
    while (!(aGenericPlayer.IsBusted()) && aGenericPlayer.IsHitting())
    {
        audio.PlaySFX("media/sounds/SFX/Card_Dealt.ogg", 0.2f, 0.4f, -2.0f, 2.0f, aGenericPlayer.pos, aGenericPlayer.vel);
        Deal(aGenericPlayer);
        cout << aGenericPlayer << endl;

        if (aGenericPlayer.IsBusted())
        {
            aGenericPlayer.Bust();
        }
    }
}

Game::Game(const vector<string>& names)
{
    //create a vector of players from a vector of names       
    vector<string>::const_iterator pName;
    for ( unsigned int i = 0; i < names.size(); i++)
    //for (pName = names.begin(); pName != names.end(); ++pName)
    {

        m_Players.push_back(Player(names[i], i, names.size()));
    }

    //seed the random number generator
    srand(static_cast<unsigned int>(time(0)));
    m_Deck.Populate();
    m_Deck.Shuffle();
}

Game::~Game()
{}

void Game::Play(AudioManager& audio)
{


    //deal initial 2 cards to everyone
    FMOD_VECTOR dealerPos = { 1.2f, -0.5f, 0.0f };
    FMOD_VECTOR dealerVel = { 0.0f, 0.0f, 0.0f };
    audio.PlaySFX("media/sounds/SFX/Card_Shuffle_Full.ogg", 0.4f, 0.5f, -1.4f, 1.4f, dealerPos, dealerVel);
    this_thread::sleep_for(chrono::seconds(6));

    vector<Player>::iterator pPlayer;
    for (int i = 0; i < 2; ++i)
    {
        for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer)
        {
            m_Deck.Deal(*pPlayer);
        }
        m_Deck.Deal(m_House);
    }

    //hide house's first card
    m_House.FlipFirstCard();

    //display everyone's hand
    FMOD_VECTOR playerPos;
    FMOD_VECTOR playerVel;
    for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer)
    {
        playerPos = pPlayer->getPos();
        playerVel = pPlayer->getVel();
        audio.PlaySFX("media/sounds/SFX/Card_Dealt_2.ogg", 0.6f, 1.0f, -6.0f, 6.0f, playerPos, playerVel);
        this_thread::sleep_for(chrono::seconds(2));
        cout << *pPlayer << endl;
    }
    audio.PlaySFX("media/sounds/SFX/Card_Dealt_2.ogg", 0.6f, 1.0f, -6.0f, 6.0f, dealerPos, dealerVel);
    this_thread::sleep_for(chrono::seconds(2));
    cout << m_House << endl;

    //deal additional cards to players
    for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer)
    {
        m_Deck.AdditionalCards(*pPlayer, audio);
        this_thread::sleep_for(chrono::seconds(1));
    }

    //reveal house's first card
    m_House.FlipFirstCard();
    cout << endl << m_House;

    //deal additional cards to house
    m_Deck.AdditionalCards(m_House, audio);
    this_thread::sleep_for(chrono::seconds(2));

    if (m_House.IsBusted())
    {
        //everyone still playing wins
        for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer)
        {
            if (!(pPlayer->IsBusted()))
            {
                playerPos = pPlayer->getPos();
                playerVel = pPlayer->getVel();
                audio.PlaySFX("media/sounds/SFX/Clank.ogg", 0.4f, 0.8f, 1.0f, 6.0f, playerPos, playerVel);
                pPlayer->Win();
                this_thread::sleep_for(chrono::seconds(1));
            }
        }
    }
    else
    {
        //compare each player still playing to house
        for (pPlayer = m_Players.begin(); pPlayer != m_Players.end();
            ++pPlayer)
        {
            if (!(pPlayer->IsBusted()))
            {
                if (pPlayer->GetTotal() > m_House.GetTotal())
                {
                    playerPos = pPlayer->getPos();
                    playerVel = pPlayer->getVel();
                    audio.PlaySFX("media/sounds/SFX/Clank.ogg", 0.4f, 0.8f, 1.0f, 6.0f, playerPos, playerVel);
                    pPlayer->Win();
                    this_thread::sleep_for(chrono::seconds(1));
                }
                else if (pPlayer->GetTotal() < m_House.GetTotal())
                {
                    pPlayer->Lose();
                    this_thread::sleep_for(chrono::seconds(1));
                }
                else
                {
                    pPlayer->Push();
                    this_thread::sleep_for(chrono::seconds(1));
                }
            }
        }

    }

    //remove everyone's cards
    for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer)
    {
        pPlayer->Clear();
    }
    m_House.Clear();
}

// MAIN
//////////
ostream& operator<<(ostream& os, const Card& aCard);
ostream& operator<<(ostream& os, const GenericPlayer& aGenericPlayer);

int main()
{
    // Elapsed time
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();


    // Audio Initialization
    AudioManager audio;
    audio.SetMasterVolume(1.0f);
    audio.SetSongsVolume(0.6f);
    audio.SetSFXsVolume(1.0f);

    // Load Music and SFX
    audio.LoadSong("media/sounds/songs/Casino_Ambience.ogg");
    audio.LoadSFX("media/sounds/SFX/Card_Dealt.ogg");
    audio.LoadSFX("media/sounds/SFX/Card_Dealt_2.ogg");
    audio.LoadSFX("media/sounds/SFX/Card_Shuffle_Full.ogg");
    audio.LoadSFX("media/sounds/SFX/Clank.ogg");

    cout << "\n\t\tBLACKJACK CASINO\n\n";

    string inputLine = "x";
    cout << "\n*A security guard twice your height blocks your way*\n-Would you like to ENTER Blackjack Casino?\n";

    while (!inputLine.size() == 0)
    {
        cout << "\n*The security guard stares at you*\n";
        getline(cin, inputLine);
    }

    audio.PlaySong("media/sounds/songs/Casino_Ambience.ogg");
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    audio.Update(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
    begin = std::chrono::steady_clock::now();

    int numPlayers = 0;

    cout << "\n*A middle age woman approaches*\n-Please, follow me we. We will open a new table for you.\n";
    cout << "\n*She is walking you to a VIP table with seven seats*\n";

    while (numPlayers < 1 || numPlayers > 7)
    {
        cout << "-How many players will sit to the table?\n";
        cin >> numPlayers;
        if (cin.fail())
        {
            cin.clear(); // reset failbit
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cout << "-Excuse me? I do not understand you.\n";
        }
    }

    vector<string> names;
    string name;
    for (int i = 0; i < numPlayers; ++i)
    {
        if (i == 0)
        {
            cout << "\n*She points to the first person*\n-What is your name?\n";
        }
        else
        {
            cout << "\n*She points to the next person*\n-What is your name?\n";
        }
        cin >> name;
        names.push_back(name);
    }
    cout << "\n*Everyone is sitting to the table*\n\n";

    //the game loop
    Game aGame(names);
    char again = 'y';
    while (again != 'n' && again != 'N')
    {
        aGame.Play(audio);
        audio.Update(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
        begin = std::chrono::steady_clock::now();
        cout << "\nDo you want to continue playing? (Y/N):";
        cin >> again;
    }

    audio.StopSongs();
    audio.StopSFXs();
    return 0;
}

//overloads << operator so Card object can be sent to cout
ostream& operator<<(ostream& os, const Card& aCard)
{
    const string RANKS[] = { "0", "A", "2", "3", "4", "5", "6", "7", "8", "9",
                            "10", "J", "Q", "K" };
    const string SUITS[] = { "c", "d", "h", "s" };

    if (aCard.m_IsFaceUp)
    {
        os << RANKS[aCard.m_Rank] << SUITS[aCard.m_Suit];
    }
    else
    {
        os << "XX";
    }

    return os;
}

//overloads << operator so a GenericPlayer object can be sent to cout
ostream& operator<<(ostream& os, const GenericPlayer& aGenericPlayer)
{
    os << aGenericPlayer.m_Name << ":\t";

    vector<Card*>::const_iterator pCard;
    if (!aGenericPlayer.m_Cards.empty())
    {
        for (pCard = aGenericPlayer.m_Cards.begin();
            pCard != aGenericPlayer.m_Cards.end();
            ++pCard)
        {
            os << *(*pCard) << "\t";
        }


        if (aGenericPlayer.GetTotal() != 0)
        {
            cout << "(" << aGenericPlayer.GetTotal() << ")";
        }
    }
    else
    {
        os << "<empty>";
    }

    return os;
}

