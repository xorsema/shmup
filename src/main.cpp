/*
  The MIT License (MIT)

  Copyright (c) 2014 Max Rose

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <iostream>

#define FPS 60
#define CROSSHAIR_FILE "res/crosshair.png"

//function decls
void die( std::string msg );

//foward decls
struct entity;
struct player;
struct enemy;
struct crosshair;
struct bullet;

//class/struct defs
struct entity
{
	sf::Vector2f vel;
	virtual sf::Drawable &getDrawable() = 0;
	virtual sf::FloatRect getGlobalBounds() = 0;
};

struct player: entity
{
	player();
	player( sf::Vector2f );
	virtual sf::Drawable &getDrawable();
	virtual sf::FloatRect getGlobalBounds();
	bool canFire();
	void fireBullet( sf::Vector2f dir );
	sf::RectangleShape shape;
	std::vector<bullet*> bullets;
	int firingSpeed;
	double speed;
};

struct enemy: entity
{

};

struct crosshair: entity
{
	crosshair();
	virtual sf::Drawable &getDrawable();
	virtual sf::FloatRect getGlobalBounds();
	sf::Texture texture;
	sf::Sprite sprite;
};

struct bullet: entity
{
	bullet( entity* parent );
	bool shouldBeDestroyed();
	virtual sf::Drawable &getDrawable();
	virtual sf::FloatRect getGlobalBounds();
	sf::RectangleShape shape;
	sf::Vector2f speed;
	entity *parent;
};
//end defs


//global vars
player player;
crosshair crosshair;

int main()
{
	sf::RenderWindow window( sf::VideoMode( 800, 600 ), "shmup" );
	sf::Event event;
	window.setFramerateLimit( FPS );
	window.setMouseCursorVisible( false );

	while( window.isOpen() )
	{
		while( window.pollEvent( event ) )
		{
			if( event.type == sf::Event::Closed )
				window.close();

			
		}

		player.shape.move( player.vel );

		if ( sf::Mouse::isButtonPressed( sf::Mouse::Left ) && player.canFire() )
		{
			sf::Vector2f v = crosshair.sprite.getPosition() - player.shape.getPosition();
			double len = sqrt( (v.x*v.x) + (v.y*v.y) );
			v.x /= len;
			v.y /= len;
			player.fireBullet( v );
		}

		window.clear( sf::Color::Black );

		crosshair.sprite.setPosition( window.mapPixelToCoords( sf::Mouse::getPosition(window) ) );
		
		window.draw( player.getDrawable() );

		for( auto it = player.bullets.begin(); it != player.bullets.end(); it++ )
		{
			(*it)->shape.move( (*it)->vel );
			window.draw( (*it)->getDrawable() );
		}

		window.draw( crosshair.getDrawable() );
		
		window.display();
	}
	
	return 0;
}

void die( std::string msg )
{
	std::cout << "Fatal error: \"" << msg << "\" quiting...\n";
	std::exit( EXIT_FAILURE );
}

//class methods
player::player()
{
	this->shape = sf::RectangleShape( sf::Vector2f( 25, 25 ) );
	speed = 30;
	firingSpeed = 200;
}

player::player( sf::Vector2f pos )
{
	player();
	this->shape.setPosition( pos );
}

sf::Drawable &player::getDrawable()
{
	return this->shape;
}

sf::FloatRect player::getGlobalBounds()
{
	return this->shape.getGlobalBounds();
}

crosshair::crosshair()
{
	if( !this->texture.loadFromFile( CROSSHAIR_FILE ) )
	{
		die( "Could not load crosshair image!" );
	}

	this->sprite.setTexture( this->texture );
	this->sprite.setOrigin( this->sprite.getLocalBounds().width / 2, this->sprite.getLocalBounds().height / 2 );
}

sf::Drawable &crosshair::getDrawable()
{
	return this->sprite;
}

sf::FloatRect crosshair::getGlobalBounds()
{
	return this->sprite.getGlobalBounds();
}

bullet::bullet( entity* parent )
{
	this->parent = parent;
	this->speed.x = this->speed.y = 10.0;
	this->shape = sf::RectangleShape( sf::Vector2f( 5, 5 ) );
	this->shape.setOrigin( this->shape.getLocalBounds().width / 2, this->shape.getLocalBounds().height / 2 );
}

sf::Drawable &bullet::getDrawable()
{
	return this->shape;
}

sf::FloatRect bullet::getGlobalBounds()
{
	return this->shape.getGlobalBounds();
}

bool player::canFire()
{
	static sf::Clock clock;
	sf::Time time;
	if( ( time = clock.getElapsedTime() ).asMilliseconds() <= this->firingSpeed )
	{
		return false;
	}
	else if ( time.asMilliseconds() > this->firingSpeed )
	{
		clock.restart();
		return true;
	}
	return false;
}

void player::fireBullet( sf::Vector2f dir )
{
	bullet *b = new bullet( this );
	b->vel.x = dir.x * b->speed.x;
	b->vel.y = dir.y * b->speed.y;
	this->bullets.push_back( b );
}

//end class methods
