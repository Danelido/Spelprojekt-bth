#include "PlayState.h"
#include "Game.h"
#include "Math.h"
#include "Utility.h"
#include "Math.h"
#include "ScoreState.h"
#include <iostream>
#include "ResourceManager.h"

PlayState::PlayState(Game * game) :	State(game)
{
	this->background.setSize((sf::Vector2f)this->game->getWindow()->getSize());
	this->background.setTexture(&ResourceManager::getTexture("Background"));

	this->particleHandler = new ParticleHandler();
	this->player = new Player(32.f, 100.f, this->game->getWindow()->getSize(), this->particleHandler);
	this->player->setPosition((sf::Vector2f)this->game->getWindow()->getSize()/2.f);
	this->obstacleHandler = new ObstacleHandler(this->player, this->particleHandler, this->game->getWindow()->getSize());
	this->projectionHandler = new ProjectileHandler(this->obstacleHandler, this->particleHandler, this->game->getWindow()->getSize());
	this->isShooting = false;
	this->fireRate = 0.045f; // less = faster shooting | more = slower shooting
	this->fireTimer = 0.0f;
	
	this->playerScoreText.setPosition(sf::Vector2f(10.f, 10.f));
	this->destroyedObstaclesText.setPosition(sf::Vector2f(10.f, 40.f));
	this->gameInfo.setText("Press H to start!");
	this->gameInfo.setTextSize(35);
	this->gameInfo.centerWithinBounds(sf::FloatRect(0.f, this->game->getWindow()->getSize().y / 2.f - 250,
		(float)this->game->getWindow()->getSize().x, this->game->getWindow()->getSize().y / 2.f - 100));
	this->gameCountdown = 3.f;
	this->startCountdown = false;
	this->canAddEnemies = true;
}	

PlayState::~PlayState()
{
	delete this->player;
	delete this->projectionHandler;
	delete this->obstacleHandler;
	delete this->particleHandler;
}

void PlayState::input()
{
	// getting mouse position relative to the window
	this->mousePos = (sf::Vector2f)sf::Mouse::getPosition(*this->game->getWindow());
	
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		this->player->shouldMove(true).setDirectionMultiplier(CustomMath::getDifferenceNormalized(this->player->getPosition(), this->mousePos));
		
	}
	else
	{
		this->player->shouldMove(false);
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	{
		this->isShooting = true;
	}
	else
	{
		this->isShooting = false;
	}

	// Start game
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::H))
	{
		if (this->startCountdown == false && this->gameCountdown > 0.0f)
		{
			this->startCountdown = true;
		}
	}
}

void PlayState::update(float dt)
{	
		this->handleShooting(dt);
		this->obstacleHandler->update(dt);
		this->projectionHandler->update(dt);
		this->particleHandler->update(dt);
		this->player->setAngle(CustomMath::getAngle(this->player->getPosition(), this->mousePos) + 90.f); // adding 90 degrees to make the ship rotate towars the mouse with one of the corners
		this->player->update(dt);
		this->handleGameinfo(dt);
		
		if (this->gameCountdown <= 0.0f)
		{
			this->playLogic(dt);
		}

		this->playerScoreText.setText("Score: " + std::to_string(this->player->getScore()));
		this->destroyedObstaclesText.setText("Destroyed objects: " + std::to_string(this->obstacleHandler->getNrOfDestroyedObstacles()));

		if (this->player->getHealth() <= 0)
		{
			this->game->changeState(new ScoreState(this->game,this->player->getScore()));
		}
}

void PlayState::render()
{
	this->game->getWindow()->draw(this->background);
	this->projectionHandler->render(this->game->getWindow());
	this->obstacleHandler->render(this->game->getWindow());
	this->particleHandler->render(this->game->getWindow());
	this->player->render(this->game->getWindow());

	this->game->getWindow()->draw(this->playerScoreText.getDrawable());
	this->game->getWindow()->draw(this->destroyedObstaclesText.getDrawable());
	
	if (this->gameCountdown > 0.0f)
	{
		this->game->getWindow()->draw(this->gameInfo.getDrawable());
	}
}

void PlayState::updateViewport()
{
	this->player->updateViewport(this->game->getWindow()->getSize());
	this->obstacleHandler->updateViewport(this->game->getWindow()->getSize());
	this->projectionHandler->updateViewport(this->game->getWindow()->getSize());
	this->background.setSize((sf::Vector2f)this->game->getWindow()->getSize());
	// reposition game info text to the center of the screen
	this->gameInfo.centerWithinBounds(sf::FloatRect(0.f, this->game->getWindow()->getSize().y / 2.f - 250,
		(float)this->game->getWindow()->getSize().x, this->game->getWindow()->getSize().y / 2.f - 100));
}

void PlayState::handleShooting(float dt)
{
	if (this->isShooting)
	{
		this->fireTimer -= dt;
		if (this->fireTimer <= 0.0f)
		{
			this->fireTimer = this->fireRate;
			this->projectionHandler->addProjectile(this->player->getPosition(),
				CustomMath::getDifferenceNormalized(this->player->getPosition(), this->mousePos));
		}
	}
	else
	{
		if (this->fireTimer > 0.0f)
		{
			this->fireTimer -= dt;
			if (this->fireTimer <= 0.0f)
			{
				this->fireTimer = 0.0f;
			}
		}
	}
}

void PlayState::playLogic(float dt)
{
	this->player->incrementScore(dt);

	if (this->player->getScore() % 30 == 0)
	{
		if (this->canAddEnemies)
		{
			this->obstacleHandler->addCircleObstacle();
			this->obstacleHandler->addTriangleObject();
			this->obstacleHandler->addRectangleObstacle();
			
			this->canAddEnemies = false;
		}
	}
	else
	{
		if (!this->canAddEnemies)
		{
			this->canAddEnemies = true;
		}
	}
}

void PlayState::handleGameinfo(float dt)
{
	if (this->gameCountdown >= 0.0f && this->startCountdown)
	{
		this->gameInfo.setText("Game starts in " + std::to_string(static_cast<int>(ceil(this->gameCountdown))));
		this->gameInfo.centerWithinBounds(sf::FloatRect(0.f, this->game->getWindow()->getSize().y / 2.f - 250,
			(float)this->game->getWindow()->getSize().x, this->game->getWindow()->getSize().y / 2.f - 100));
		this->gameCountdown -= dt;
	}
	else if(this->gameCountdown <= 0 && this->startCountdown)
	{
		this->gameCountdown = 0.0f;
		this->startCountdown = false;
	}
}

