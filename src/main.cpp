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
#include <list>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>

#define FPS 60
#define PI 3.14159265
#define CROSSHAIR_FILE "res/crosshair.png"
#define TITLE_FILE "res/title.png"
#define STATE_TITLE 1
#define STATE_GAME 2

//function decls
void die( std::string msg );
void do_input( sf::RenderWindow &window );
sf::Vector2f normalize( const sf::Vector2f& );
void game_init( sf::RenderWindow& );
void game_frame( sf::RenderWindow& );
void start_screen_init( sf::RenderWindow& );
void start_screen_frame( sf::RenderWindow& );

//foward decls
struct entity;
struct player;
struct enemy;
struct crosshair;
struct bullet;
struct enemy_director;

//class/struct defs
struct entity
{
	virtual ~entity() {}
	sf::Vector2f vel;
	double speed;
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
	void reapBullets();
	sf::RectangleShape shape;
	std::list<bullet*> bullets;
	int firingSpeed;
};

struct enemy: entity
{
	enemy();
	virtual sf::Drawable &getDrawable();
	virtual sf::FloatRect getGlobalBounds();
	sf::RectangleShape shape;
	unsigned health;
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
	virtual ~bullet() {}
	virtual sf::Drawable &getDrawable();
	virtual sf::FloatRect getGlobalBounds();
	sf::RectangleShape shape;
	sf::Vector2f speed;
	entity *parent;
	sf::Time lifetime;
	sf::Clock lifeTimer;
	bool valid;
};

struct enemy_director
{
	std::list<enemy*> enemies;
	void spawnEnemy( sf::Vector2f, sf::Vector2f = sf::Vector2f( 0, 0 ) );
	void spawnEnemy( const player& );
	void handleCollisions( std::list<bullet*>& );
	void doAI( const player& );
	bool shouldSpawn();
	sf::Time spawnSpeed;
	sf::Clock spawnClock;
	double enemyMinDistance, enemyMaxDistance;
};
//end defs

//global vars
player		g_player;
crosshair	g_crosshair;
enemy_director	g_enemdir;
bool		keys[5];
bool		shouldFire;
int		game_state;
sf::Texture	titleTexture;
sf::Sprite	titleSprite;

int main()
{
	std::srand( std::time(0) );
	sf::RenderWindow window( sf::VideoMode( 800, 600 ), "shmup" );
	window.setFramerateLimit( FPS );
	window.setMouseCursorVisible( false );

	game_state = STATE_TITLE;

	game_init( window );
	start_screen_init( window );

	while( window.isOpen() )
	{	
		switch( game_state )
		{
		case STATE_GAME:
			game_frame( window );
			break;
		case STATE_TITLE:
			start_screen_frame( window );
			break;
		}
		
	}
	
	return 0;
}

void die( std::string msg )
{
	std::cout << "Fatal error: \"" << msg << "\" quiting...\n";
	std::exit( EXIT_FAILURE );
}

void do_input( sf::RenderWindow &window )
{
	sf::Event event;

	while( window.pollEvent( event ) )
	{
		switch( event.type )
		{
		case sf::Event::Closed:
			window.close();
			break;

		case sf::Event::KeyPressed:
			switch( event.key.code )
			{
			case sf::Keyboard::W:
				keys[0] = true;
				break;
			case sf::Keyboard::A:
				keys[1] = true;
				break;
			case sf::Keyboard::S:
				keys[2] = true;
				break;
			case sf::Keyboard::D:
				keys[3] = true;
				break;
			case sf::Keyboard::Return:
				keys[4] = true;
				break;
			}
			break;

		case sf::Event::KeyReleased:
			switch( event.key.code )
			{
			case sf::Keyboard::W:
				keys[0] = false;
				break;
			case sf::Keyboard::A:
				keys[1] = false;
				break;
			case sf::Keyboard::S:
				keys[2] = false;
				break;
			case sf::Keyboard::D:
				keys[3] = false;
				break;
			case sf::Keyboard::Return:
				keys[4] = false;
				break;
			}
			break;

		case sf::Event::MouseButtonPressed:
			switch( event.mouseButton.button )
			{
			case sf::Mouse::Left:
				shouldFire = true;
				break;
			}
			break;

		case sf::Event::MouseButtonReleased:
			switch( event.mouseButton.button )
			{
			case sf::Mouse::Left:
				shouldFire = false;
				break;
			}
			break;
				
		case sf::Event::MouseMoved:
			g_crosshair.sprite.setPosition( window.mapPixelToCoords( sf::Vector2i( event.mouseMove.x, event.mouseMove.y ) ) );
						
		}
			
	}
}

sf::Vector2f normalize( const sf::Vector2f& v )
{
	sf::Vector2f r = v;
	double len = sqrt( (v.x * v.x) + (v.y * v.y) );
	r.x /= len;
	r.y /= len;

	return r;
}

void game_init( sf::RenderWindow& window )
{
	g_player.shape.setPosition( ((float)window.getSize().x)/2, ((float)window.getSize().y)/2 );

	g_player.canFire();

	g_enemdir.enemyMinDistance = 100.0;
	g_enemdir.enemyMaxDistance = 300.0;
	g_enemdir.spawnSpeed = sf::seconds( 2.0f );
}

void game_frame( sf::RenderWindow& window )
{
	do_input( window );

	if( !keys[0] && !keys[2] )
		g_player.vel.y = 0;
	if( !keys[1] && !keys[3] )
		g_player.vel.x = 0;
	
	if( keys[0] ) g_player.vel.y = -g_player.speed;
	if( keys[1] ) g_player.vel.x = -g_player.speed;
	if( keys[2] ) g_player.vel.y = g_player.speed;
	if( keys[3] ) g_player.vel.x = g_player.speed;
	
	if( g_enemdir.shouldSpawn() )
		g_enemdir.spawnEnemy( g_player );
	
	g_enemdir.handleCollisions( g_player.bullets );
	
	g_player.reapBullets();
	
	g_player.shape.move( g_player.vel );
	
	g_enemdir.doAI( g_player );
	
	if ( shouldFire && g_player.canFire() )
	{
		sf::Vector2f v = g_crosshair.sprite.getPosition() - g_player.shape.getPosition();
		double len = sqrt( (v.x*v.x) + (v.y*v.y) );
		v.x /= len;
		v.y /= len;
		g_player.fireBullet( v );
	}
	
	window.clear( sf::Color::Black );
	
	window.draw( g_player.getDrawable() );
	
	for( auto it = g_player.bullets.begin(); it != g_player.bullets.end(); it++ )
	{
		(*it)->shape.move( (*it)->vel );
		window.draw( (*it)->getDrawable() );
	}
	
	for( auto it = g_enemdir.enemies.begin(); it != g_enemdir.enemies.end(); it++ )
	{
		(*it)->shape.move( (*it)->vel );
		window.draw( (*it)->getDrawable() );
	}
	
	window.draw( g_crosshair.getDrawable() );
	
	window.display();
}
void start_screen_init( sf::RenderWindow& window )
{
	if( !titleTexture.loadFromFile( TITLE_FILE ) )
	{
		die( "couldn't open title image!" );
	}

	titleSprite.setTexture( titleTexture );
}

void start_screen_frame( sf::RenderWindow& window )
{
	do_input( window );

	if( keys[4] )
	{
		game_state = STATE_GAME;
	}

	window.clear( sf::Color::Black );
	window.draw( titleSprite );
	window.display();
}

//class methods
//player
player::player()
{
	this->shape = sf::RectangleShape( sf::Vector2f( 25, 25 ) );
	this->shape.setOrigin( this->shape.getLocalBounds().width / 2, this->shape.getLocalBounds().height / 2 );
	speed = 2;
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
	
	b->shape.setPosition( this->shape.getPosition() );
	
	this->bullets.push_back( b );
}

void player::reapBullets()
{
	auto it = this->bullets.begin();
	while( it != this->bullets.end() )
	{
		if( (*it)->lifeTimer.getElapsedTime() >= (*it)->lifetime || !( (*it)->valid ) ) 
		{
			auto del = it;
			it++;
			bullet *ptr = *del;
			this->bullets.erase( del );
			delete ptr;
		}
		else
			it++;
	}
}

//crosshair
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

//bullet
bullet::bullet( entity* parent )
{
	this->parent = parent;
	this->speed.x = this->speed.y = 10.0;
	this->shape = sf::RectangleShape( sf::Vector2f( 5, 5 ) );
	this->shape.setOrigin( this->shape.getLocalBounds().width / 2, this->shape.getLocalBounds().height / 2 );
	this->lifetime = sf::seconds( 5.0f );
	this->valid = true;
}

sf::Drawable &bullet::getDrawable()
{
	return this->shape;
}

sf::FloatRect bullet::getGlobalBounds()
{
	return this->shape.getGlobalBounds();
}

//enemy
enemy::enemy()
{
	this->shape = sf::RectangleShape( sf::Vector2f( 25, 25 ) );
	this->shape.setFillColor( sf::Color( 255, 0, 0 ) );
	this->shape.setOrigin( this->shape.getLocalBounds().width / 2, this->shape.getLocalBounds().height / 2 );
	this->health = 5;
	this->speed = 1.0;
}

sf::Drawable &enemy::getDrawable()
{
	return this->shape;
}

sf::FloatRect enemy::getGlobalBounds()
{
	return this->shape.getGlobalBounds();
}

//enemy_director
void enemy_director::spawnEnemy( sf::Vector2f pos, sf::Vector2f vel )
{
	enemy *e = new enemy;
	e->shape.setPosition( pos );
	e->vel = vel;
	this->enemies.push_back( e );
}

void enemy_director::spawnEnemy( const player& p )
{
	double angle = static_cast<double>( rand() ) / ( static_cast<double>( RAND_MAX / ( PI*2 ) ) );
	double distance = this->enemyMinDistance + static_cast<double>( rand() ) / ( static_cast<double>( RAND_MAX / ( this->enemyMaxDistance - this->enemyMinDistance ) ) );
	sf::Vector2f pos( std::cos( angle ) * distance, std::sin( angle ) * distance );
	this->spawnEnemy( pos + p.shape.getPosition() );
}

void enemy_director::handleCollisions( std::list<bullet*>& bullets  )
{
	for( auto i = bullets.begin(); i != bullets.end(); i++ )
	{
		if( (*i)->valid )
		{
			for( auto j = this->enemies.begin(); j != enemies.end(); )
			{
				if( (*i)->getGlobalBounds().intersects( (*j)->getGlobalBounds() ) )
				{
					(*i)->valid = false;
					if( (*j)->health == 0 )
					{
						auto del = j;
						enemy *ptr = *del;
						j++;
						enemies.erase( del );
						delete ptr;
					
					}
					else
					{
						(*j)->health--;
						j++;
					}
				}
				else
					j++;
			}
		}
	}
}

void enemy_director::doAI( const player &p )
{
	for( auto it = this->enemies.begin(); it != this->enemies.end(); it++ )
	{
		sf::Vector2f dir_to_player = p.shape.getPosition() - (*it)->shape.getPosition();
		dir_to_player = normalize( dir_to_player );
		sf::Vector2f vel( dir_to_player.x * (*it)->speed, dir_to_player.y * (*it)->speed );
		(*it)->vel = vel;
	}
}

bool enemy_director::shouldSpawn()
{
	if( spawnClock.getElapsedTime() < spawnSpeed )
		return false;

	spawnClock.restart();
	return true;
}

//end class methods
