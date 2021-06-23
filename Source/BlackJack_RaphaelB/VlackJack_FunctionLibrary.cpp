// Fill out your copyright notice in the Description page of Project Settings.


#include "VlackJack_FunctionLibrary.h"

/*

By Jason Wood & Andreas Carlen
4/12/2021


*/

#include <iostream>
#include <random>
#include <cstdint>

using namespace std;

constexpr size_t    NUM_SUITS = 4,
NUM_CARDS_PER_SUIT = 13,
NUM_CARDS_PER_DECK = NUM_SUITS * NUM_CARDS_PER_SUIT,
MAX_NUM_CARDS_PER_HAND = 11;

struct Hand {
    uint8_t count = 0;
    uint8_t cards[MAX_NUM_CARDS_PER_HAND];
};

struct GameState {
    Hand dealerHand, playerHand;
    int playerWallet = 100, betAmount = 0;
};

////////////////////////////////////////////////////////////////////////////////
// some std::random globals

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> distrib(0, NUM_CARDS_PER_DECK - 1);


////////////////////////////////////////////////////////////////////////////////
// some utility functions

void hit(Hand& h) {
    h.cards[h.count++] = static_cast<uint8_t>(distrib(gen));
}

constexpr uint8_t cardFace(uint8_t card) {
    return card % NUM_CARDS_PER_SUIT;
}

constexpr int cardValue(uint8_t face) {
    return face < 8 ? face + 2 : face < 12 ? 10 : 11;
}

int handValue(Hand const& h) {
    int tot{ 0 }, aceCount{ 0 };

    for (auto i = 0; i < h.count; i++) {
        auto const val = cardValue(cardFace(h.cards[i]));
        if (val == 11) aceCount++;
        else tot += val;
    }

    tot += aceCount;
    if (aceCount && tot <= 11)
        tot += 10;
    return tot;
}

void printCard(uint8_t c) {
    static const char suitNames[] = { 'H', 'D', 'C', 'S', }, faceNames[] = { 'J', 'Q', 'K', 'A', };

    int const suit = c / NUM_CARDS_PER_SUIT, face = cardFace(c);

    if (face <= 8)
        cout << face + 2;
    else cout << faceNames[face - 9];

    cout << suitNames[suit];
}

void printHand(const char* pre, Hand const& h, bool showFirst = true) {
    cout << pre;

    if (showFirst) printCard(h.cards[0]);
    else cout << "??";

    for (auto i = 1; i < h.count; i++) {
        cout << ' ';
        printCard(h.cards[i]);
    }
}

////////////////////////////////////////////////////////////////////////////////
// more interesting stuff...

char getPlayerChoice(GameState& state, bool first = false) {
    while (true) {
        char ch;

        cout << "*** Your Choices: s(t)and / (h)it";

        if (first) {
            cout << " / su(r)render / (d)ouble down";
        }

        cout << "\n>";
        cin >> ch;

        if (first) {
            if (ch == 'r') {
                cout << "You surrender.\n";
                state.playerWallet += state.betAmount / 2; // player gets back 1/2 his bet
                return ch;
            }
            else if (ch == 'd') {
                if (state.playerWallet < state.betAmount) {
                    cout << "You don't have enough funds.\n";
                }
                else {
                    cout << "You double down.\n";

                    // player doubles his bet
                    state.playerWallet -= state.betAmount;
                    state.betAmount *= 2;
                    hit(state.playerHand); // ... and gets one more card dealt to his hand
                    return ch;
                }
            }
        }

        if (ch == 't') {
            cout << "You stand.\n";
            return ch;
        }
        else if (ch == 'h') {
            cout << "You hit.\n";
            hit(state.playerHand);
            return ch;
        }
        else {
            cout << "Huh?\n";
        }
    }
}

void regularPlay(GameState& state) {
    char ch = getPlayerChoice(state, true);

    if (ch == 'r') // surrender?
        return;

    int pv, dv;

    do {
        pv = handValue(state.playerHand);

        if (ch == 'd' || ch == 'h') { // double down or hit?
            printHand("Your hand: ", state.playerHand);
            cout << "\t\t(Value = " << pv << ")\n";
        }

        if (pv > 21) {
            cout << "You bust.\n";
            return;
        }
        else if (pv == 21 || ch == 'd' || ch == 't') {
            break;
        }

        ch = getPlayerChoice(state);
    } while (true);

    //

    cout << "\nDealer's turn...\n";

    do {
        dv = handValue(state.dealerHand);

        printHand("Dealer's hand: ", state.dealerHand);
        cout << "\t\t(Value = " << dv << ")\n";

        if (dv >= 17) break;

        cout << "Dealer hits.\n";
        hit(state.dealerHand);
    } while (true);

    // handle win/lose + payouts

    if (dv > 21 || dv < pv) {
        cout << "You win!\n";
        state.playerWallet += 2 * state.betAmount;
    }
    else if (dv > pv) {
        cout << "Dealer wins.\n";
    }
    else {
        cout << "It's a tie.\n";
        state.playerWallet += state.betAmount;
    }
}

void initialDeal(GameState& state) {
    auto& dh = state.dealerHand, & ph = state.playerHand;

    hit(ph); hit(dh);
    hit(ph); hit(dh);

    auto pv = handValue(ph), dv = handValue(dh);

    printHand("Your hand: ", ph);
    cout << "\t\t(Value = " << pv << ")\n";

    printHand("Dealer's hand: ", dh, false);
    cout << "\t\t(Value = ?? )\n\n";

    // check for Player and/or Dealer Blackjack
    if (pv == 21) {
        state.playerWallet += state.betAmount; // player gets back his bet

        if (dv == 21) {
            cout << "You and the Dealer both have Blackjack! It's a tie.\n";
        }
        else {
            cout << "You have Blackjack! You win!!\n";
            state.playerWallet += 3 * state.betAmount / 2; // ... + 3/2 bet
        }

        printHand("Dealer's hand: ", dh);
        cout << "\t\t(Value = " << dv << ")\n";
    }
    else if (dv == 21) {
        cout << "Dealer has Blackjack!\n";

        printHand("Dealer's hand: ", dh);
        cout << "\t\t(Value = " << dv << ")\n";
    }
    else {
        regularPlay(state);
    }
}

int main() {
    int betAmount = 10;
    GameState state;

    while (true) {
        cout << "\n*************************\nYour wallet: $" << state.playerWallet << '\n';

        if (state.playerWallet < betAmount) {
            cout << "You're broke! Cya.\n";
            break;
        }

        cout << "Press (b) to bet $" << betAmount << " or any other key to quit.\n>";

        char k;
        cin >> k;
        if (k != 'b') {
            cout << "Bye.\n";
            break;
        }

        state.playerWallet -= betAmount;
        state.betAmount = betAmount;
        state.dealerHand = state.playerHand = Hand();

        initialDeal(state);
    }
    return 0;
}
