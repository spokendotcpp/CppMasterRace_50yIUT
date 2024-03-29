#include <osgViewer/Viewer>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osg/Geometry>
#include <osg/PositionAttitudeTransform>
#include <osgGA/NodeTrackerManipulator>
#include <osgGA/GUIEventHandler>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/DriveManipulator>
#include <osgSim/DOFTransform>
#include <osg/AnimationPath>
#include <osgParticle/SmokeEffect>
#include <string>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include "renderToTexture.h"
//#include "fpsCamera.h"

osg::ref_ptr<osg::Group> root;
osgViewer::Viewer viewer;
osg::ref_ptr<osg::Group> scene;

osg::ref_ptr<osg::Geometry> quadSol;
osg::ref_ptr<osg::Geometry> quadCiel;
osg::ref_ptr<osg::Texture2D> textureSol;
osg::ref_ptr<osg::Texture2D> textureCiel;

osg::ref_ptr<osg::PositionAttitudeTransform> transformFeet;
osg::ref_ptr<osg::PositionAttitudeTransform> patSpeed;
osg::ref_ptr<osg::Node> feet;

osg::ref_ptr<osg::Geode> geodeSol;
osg::ref_ptr<osg::Geode> geodeCiel;

osg::ref_ptr<osgGA::DriveManipulator> manip;

float fieldX = 1000.0;
float fieldY = 1000.0;


class Barette : public osg::PositionAttitudeTransform{
public:
	Barette(float _angle);
	osg::Vec2 vit;
};

Barette::Barette(float _angle){
	vit.x() = cos(_angle)/10;
	vit.y() = sin(_angle)/10;
}

float anglePiedG = 0.0;

class WalkPiedG : public osg::NodeCallback
{
public:
    virtual void operator() (osg::Node* n, osg::NodeVisitor* nv)
    {
		osg::PositionAttitudeTransform* pos = (osg::PositionAttitudeTransform*)n;
		static bool monte = true;
		if(monte){
			anglePiedG -= 0.02;
			if(anglePiedG<-50) monte = false;
		}
		if(!monte){
			anglePiedG += 0.02;
			if(anglePiedG>50) monte = true;
		}
		pos->setAttitude(osg::Quat(osg::DegreesToRadians(anglePiedG), osg::Vec3(1.0, 0.0, 0.0)));

		viewer.getCamera()->setViewMatrixAsLookAt( osg::Vec3d(1000.0, 1000.0, 0.0), osg::Vec3d(0.0,0.0,0.0), osg::Vec3d(0.0, 0.0, 1.0) );

    }
};

float anglePiedD = 0.0;

class WalkPiedD : public osg::NodeCallback
{
public:
    virtual void operator() (osg::Node* n, osg::NodeVisitor* nv)
    {
		osg::PositionAttitudeTransform* pos = (osg::PositionAttitudeTransform*)n;
		static bool monte = true;
		if(monte){
			anglePiedD += 0.02;
			if(anglePiedD>50) monte = false;
		}
		if(!monte){
			anglePiedD -= 0.02;
			if(anglePiedD<-50) monte = true;
		}
		pos->setAttitude(osg::Quat(osg::DegreesToRadians(anglePiedD), osg::Vec3(1.0, 0.0, 0.0)));
    }
};

osg::Vec2 directionChikoiseau = osg::Vec2(0.0, 0.0);
float chikoiseauTimer = 0.0;

class MovementChikoiseau : public osg::NodeCallback
{
public:
    virtual void operator() (osg::Node* n, osg::NodeVisitor* nv)
    {
    	if (chikoiseauTimer > 20000.0){
    		directionChikoiseau.x() = (float((rand()%20)-10)/100.0);
    		directionChikoiseau.y() = (float((rand()%20)-10)/100.0);
    		chikoiseauTimer = 0.0;
    	}
    	chikoiseauTimer += 1.0;
		osg::PositionAttitudeTransform* pos = (osg::PositionAttitudeTransform*)n;
		pos->setPosition(osg::Vec3(pos->getPosition().x()+directionChikoiseau.x(), pos->getPosition().y()+directionChikoiseau.y(), pos->getPosition().z()));

    }
};

class voituresCallback : public osg::NodeCallback
{
public:
    virtual void operator() (osg::Node* n, osg::NodeVisitor* nv)
    {
		Barette* pos = (Barette*)n;

		pos->setPosition(osg::Vec3(pos->getPosition().x()+pos->vit.x(), pos->getPosition().y()+pos->vit.y(), pos->getPosition().z()));

		if(pos->getPosition().x() < 0 or pos->getPosition().x() > fieldX)
			pos->vit.x() = -pos->vit.x();
		else if (pos->getPosition().y() < 0 or pos->getPosition().y() > fieldY)
			pos->vit.y() = -pos->vit.y();

    }
};

float ventiradAngle = 0.0;

class ventiradCallback : public osg::NodeCallback
{
public:
    virtual void operator() (osg::Node* n, osg::NodeVisitor* nv)
    {
    	ventiradAngle += 5.0;
    	osg::PositionAttitudeTransform* pos = (osg::PositionAttitudeTransform*)n;
		pos->setAttitude(osg::Quat(osg::DegreesToRadians(ventiradAngle), osg::Vec3(1.0, 0.0, 0.0)));
    }
};

float angleAilesG = 0.0;

class FlapFlapG : public osg::NodeCallback
{
public:
    virtual void operator() (osg::Node* n, osg::NodeVisitor* nv)
    {
		osg::PositionAttitudeTransform* pos = (osg::PositionAttitudeTransform*)n;
		static bool monte = true;
		if(monte){
			angleAilesG += 0.08;
			if(angleAilesG>50) monte = false;
		}
		if(!monte){
			angleAilesG -= 0.08;
			if(angleAilesG<-50) monte = true;
		}
		pos->setAttitude(osg::Quat(osg::DegreesToRadians(angleAilesG), osg::Vec3(0.0, 1.0, 0.0)));
    }
};

float angleAilesD = 0.0;

class FlapFlapD : public osg::NodeCallback
{
public:
    virtual void operator() (osg::Node* n, osg::NodeVisitor* nv)
    {
		osg::PositionAttitudeTransform* pos = (osg::PositionAttitudeTransform*)n;
		static bool monteD = true;
		if(monteD){
			angleAilesD -= 0.08;
			if(angleAilesD<-50) monteD = false;
		}
		if(!monteD){
			angleAilesD += 0.08;
			if(angleAilesD>50) monteD = true;
		}
		pos->setAttitude(osg::Quat(osg::DegreesToRadians(angleAilesD), osg::Vec3(0.0, 1.0, 0.0)));
    }
};

float angle = 0.0;

class Rotation : public osg::NodeCallback
{
public:
    virtual void operator() (osg::Node* n, osg::NodeVisitor* nv)
    {
        // code pour modifier le nœud, par exemple la position si il s 'agit
        // d'un nœud de type osg::PositionAttitudeTransform :
        osg::PositionAttitudeTransform* pos = (osg::PositionAttitudeTransform*)n;
		angle += 0.04;
		pos->setAttitude(osg::Quat(osg::DegreesToRadians(angle), osg::Vec3(0.0, 0.0, 1.0)));
    }
};

class GestionEvenements : public osgGA::GUIEventHandler
{
	public:
	virtual bool handle( const osgGA::GUIEventAdapter& ea,
	osgGA::GUIActionAdapter& aa);
};

bool GestionEvenements::handle( const osgGA::GUIEventAdapter& ea,
 osgGA::GUIActionAdapter& aa)
{
	switch(ea.getEventType()){
        std::cout << ea.getKey() << std::endl;
			switch(ea.getKey()){
				case 'a':
					break;
				case 'z':
					break;
				case 'e':
					break;
			}
			break;

		case osgGA::GUIEventAdapter::PUSH :{
			int x = ea.getX();
			int y = ea.getY();
			break;
 }
		case osgGA::GUIEventAdapter::DOUBLECLICK :
			break;
			}
 return false; //caméra pour que l'événement soit traité par d'autres gestionnaires
}

void CreateSol(){
	textureSol = new osg::Texture2D;
	textureSol->setImage(osgDB::readImageFile("test_motherboard.jpg"));
	textureSol->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
	textureSol->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
	textureSol->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
	textureSol->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );

	quadSol = osg::createTexturedQuadGeometry(
	osg::Vec3(0.0, 0.0, 0.0), // Coin de départ
	osg::Vec3(fieldX, 0.0, 0.0),  // largeur
	osg::Vec3(0.0, fieldY, 0.0),  // hauteur
	0.0, 0.0, 30.0, 30.0); 		// Coordonnées de texture gauche/bas/droit/haut
								// Si vous mettez 4.0 à la place de 1.0,
								// la texture sera répétée 4 fois
	quadSol->getOrCreateStateSet()->setTextureAttributeAndModes(0, textureSol.get());
	quadSol->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

	geodeSol = new osg::Geode;
	geodeSol->addDrawable(quadSol);
}

void CreationCiel(){
	textureCiel = new osg::Texture2D;
	textureCiel->setImage(osgDB::readImageFile("binaire.jpg"));
	textureCiel->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
	textureCiel->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
	textureCiel->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
	textureCiel->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );

	quadCiel = osg::createTexturedQuadGeometry(
	osg::Vec3(-50000.0, -50000.0, 200.0), // Coin de départ
	osg::Vec3(100000, 0.0, 0.0),  // largeur
	osg::Vec3(0.0, 100000, 0.0),  // hauteur
	0.0, 0.0, 80.0, 80.0); 		// Coordonnées de texture gauche/bas/droit/haut
								// Si vous mettez 4.0 à la place de 1.0,
								// la texture sera répétée 4 fois
	quadCiel->getOrCreateStateSet()->setTextureAttributeAndModes(0, textureCiel.get());
	//quadCiel->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

	geodeCiel = new osg::Geode;
	geodeCiel->addDrawable(quadCiel);
}

osg::ref_ptr<osg::Group> creation_procs(int nb_procs, float taillex, float tailley){
    osg::ref_ptr<osg::Node> proc = osgDB::readNodeFile("proc.3ds");

    osg::ref_ptr<osg::Group> procs = new osg::Group;
    for(unsigned int i=0; i<= nb_procs;  ++i){
        angle = rand()%360;
        int randX = rand()%(int)taillex;
		int randY = rand()%(int)tailley;

        osg::ref_ptr<osg::PositionAttitudeTransform> tsProc = new osg::PositionAttitudeTransform();

        tsProc->setScale(osg::Vec3(10.0, 10.0, 10.0));
        tsProc->setPosition(osg::Vec3(randX, randY, 0.0));
		tsProc->setAttitude(osg::Quat(osg::DegreesToRadians(angle), osg::Vec3(0.0, 0.0, 1.0)));

        tsProc->addChild(proc);

		osg::ref_ptr<osg::PositionAttitudeTransform> theProc = new osg::PositionAttitudeTransform();
        theProc->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);

		theProc->addChild(tsProc);

		procs->addChild(theProc);
    }
    return procs;
}

osg::ref_ptr<osg::Group> creation_condens(int nb_condenss, float taillex, float tailley){
    osg::ref_ptr<osg::Node> condens = osgDB::readNodeFile("condens.3ds");

    osg::ref_ptr<osg::Group> condenss = new osg::Group;
    for(unsigned int i=0; i<= nb_condenss;  ++i){
        int randX = rand()%(int)taillex;
		int randY = rand()%(int)tailley;

        osg::ref_ptr<osg::PositionAttitudeTransform> tsCondens = new osg::PositionAttitudeTransform();

        tsCondens->setScale(osg::Vec3(10.0, 10.0, 10.0));
        tsCondens->setPosition(osg::Vec3(randX, randY, 0.0));

        tsCondens->addChild(condens);

		osg::ref_ptr<osg::PositionAttitudeTransform> theCondens = new osg::PositionAttitudeTransform();
        theCondens->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);

		theCondens->addChild(tsCondens);

		condenss->addChild(theCondens);
    }
    return condenss;
}

osg::ref_ptr<osg::Group> creation_memoryleak(int nb_memoryleak, float taillex, float tailley, std::string nameFile){
    osg::ref_ptr<osg::Group> memoryleaks = new osg::Group;
    osg::ref_ptr<osgParticle::SmokeEffect> memoryleak = new osgParticle::SmokeEffect;
	memoryleak->setTextureFileName(nameFile);
	memoryleak->setIntensity(1);
	memoryleak->setEmitterDuration(99999999.99999);

    for(unsigned int i=0; i<= nb_memoryleak; ++i){
        int randX = rand()%(int)taillex;
		int randY = rand()%(int)tailley;

        osg::ref_ptr<osg::PositionAttitudeTransform> tsMLeak = new osg::PositionAttitudeTransform();

        tsMLeak->addChild(memoryleak);
        tsMLeak->setScale(osg::Vec3(10.0, 10.0, 10.0));
        tsMLeak->setPosition(osg::Vec3(1000.0, 0.0, 0.0));

		osg::ref_ptr<osg::PositionAttitudeTransform> mleak = new osg::PositionAttitudeTransform();
        mleak->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);
		mleak->addChild(tsMLeak);
		memoryleaks->addChild(mleak);
    }

    return memoryleaks;
}

osg::ref_ptr<osg::Group> creation_lampadaires(int nb_lampadaires, float taillex, float tailley){
    osg::ref_ptr<osg::Node> lampadaire = osgDB::readNodeFile("led2.3ds");

    osg::ref_ptr<osg::Group> lampadaires = new osg::Group;
    for(unsigned int i=0; i<= nb_lampadaires; ++i){
        int randX = rand()%(int)taillex;
		int randY = rand()%(int)tailley;

        osg::ref_ptr<osg::PositionAttitudeTransform> tsLampadaire = new osg::PositionAttitudeTransform();

        tsLampadaire->setScale(osg::Vec3(1.0, 1.0, 1.0));
        tsLampadaire->setPosition(osg::Vec3(randX, randY, 0.0));

        tsLampadaire->addChild(lampadaire);

		osg::ref_ptr<osg::PositionAttitudeTransform> theLampadaire = new osg::PositionAttitudeTransform();
        theLampadaire->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);

		theLampadaire->addChild(tsLampadaire);

		lampadaires->addChild(theLampadaire);
    }
    return lampadaires;
}

osg::ref_ptr<osg::Group> creation_rams(int nb_rams, float taillex, float tailley){
    osg::ref_ptr<osg::Node> ram = osgDB::readNodeFile("ram.3ds");

    osg::ref_ptr<osg::Group> rams = new osg::Group;
    for(unsigned int i=0; i<= nb_rams; ++i){
        int randX = rand()%(int)taillex;
		int randY = rand()%(int)tailley;
		angle = rand()%360;

        osg::ref_ptr<Barette> tsRam = new Barette(angle);

        tsRam->setScale(osg::Vec3(100.0, 100.0, 100.0));
        tsRam->setAttitude(osg::Quat(osg::DegreesToRadians(angle), osg::Vec3(0.0, 0.0, 1.0)));
        tsRam->setPosition(osg::Vec3(randX, randY, 0.0));

        tsRam->setUpdateCallback(new voituresCallback);

        tsRam->addChild(ram);

		osg::ref_ptr<osg::PositionAttitudeTransform> theRam = new osg::PositionAttitudeTransform();
        theRam->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);

		theRam->addChild(tsRam);

		rams->addChild(theRam);
    }
    return rams;
}

class ChercheNoeud : public osg::NodeVisitor
{
public:
	ChercheNoeud ( const std::string& name )
	: osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ), _name( name ) {}
	// Méthode appelée pour chaque nœud du graphe. Si son nom correspond à celui passé
	// en paramètre au constructeur, on sauve l'adresse du nœud dans _node
	virtual void apply( osg::Node& node )
	{
	if (node.getName() == _name)
	_node = &node;
	traverse( node ); // On continue le parcours du graphe
	}
	osg::ref_ptr<osg::Node> getNode() { return _node.get(); }
protected:
	std::string _name;
	osg::ref_ptr<osg::Node> _node;
};

ChercheNoeud rechercheBlade("Scythe_Blade");

osg::ref_ptr<osg::Group> creation_ventirads(int nb_ventirads, float taillex, float tailley){
    osg::ref_ptr<osg::Node> ventirad = osgDB::readNodeFile("ventirad.obj");

    osg::ref_ptr<osg::Group> ventirads = new osg::Group;
    for(unsigned int i=0; i<= nb_ventirads; ++i){
        int randX = rand()%(int)taillex;
        /*if(randX < taillex/2) randX -= taillex/2;
        else randX += taillex/2;*/
		int randY = rand()%(int)tailley;
        /*if(randY < tailley/2) randY -= tailley/2;
        else randY += tailley/2;*/
		angle = rand()%360;

        osg::ref_ptr<osg::PositionAttitudeTransform> tsVentirad = new osg::PositionAttitudeTransform();

        tsVentirad->setScale(osg::Vec3(0.1, 0.1, 0.1));
        tsVentirad->setAttitude(osg::Quat(osg::DegreesToRadians(90.0), osg::Vec3(1.0, 0.0, 0.0), osg::DegreesToRadians(0.0), osg::Vec3(0.0, 1.0, 0.0), osg::DegreesToRadians(angle), osg::Vec3(0.0, 0.0, 1.0)));
        tsVentirad->setPosition(osg::Vec3(randX, randY, 0.0));

        rechercheBlade.getNode()->setUpdateCallback(new ventiradCallback);

        tsVentirad->addChild(ventirad);

        tsVentirad->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);

		ventirads->addChild(tsVentirad);
    }
    return ventirads;
}

osg::ref_ptr<osg::Group> creation_troupeau_touches(int nb_touche, float taillex, float tailley){

    osg::ref_ptr<osg::Material> material = new osg::Material();
    material->setEmission(osg::Material::FRONT, osg::Vec4(0.8, 0.8, 0.8, 1.0));

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setDataVariance(osg::Object::DYNAMIC);
    texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);

    osg::ref_ptr<osg::Image> image = osgDB::readImageFile("thon.jpeg");
    texture->setImage(image);

    osg::ref_ptr<osg::Node> feetD = osgDB::readNodeFile("feetD.obj");
    osg::ref_ptr<osg::Node> feetG = osgDB::readNodeFile("feetG.obj");
    osg::ref_ptr<osg::Node> touche = osgDB::readNodeFile("key.3ds");
    osg::ref_ptr<osg::StateSet> state = touche->getOrCreateStateSet();
    state->setTextureAttributeAndModes(0,texture.get(),osg::StateAttribute::ON);
    osg::ref_ptr<osg::Group> touches = new osg::Group;
    for(unsigned int i=0; i<= nb_touche;  ++i){
        int randX = rand()%(int)taillex;
		int randY = rand()%(int)tailley;
        float angle = rand()%360;

        //int randX =  0;
        //int randY = 500;

        osg::ref_ptr<osg::PositionAttitudeTransform> tsFeetD = new osg::PositionAttitudeTransform();
        osg::ref_ptr<osg::PositionAttitudeTransform> tsFeetG = new osg::PositionAttitudeTransform();
        osg::ref_ptr<osg::PositionAttitudeTransform> tsTouche = new osg::PositionAttitudeTransform();

        tsFeetD->setUpdateCallback(new WalkPiedD);
		tsFeetG->setUpdateCallback(new WalkPiedG);

        tsTouche->setScale(osg::Vec3(0.2, 0.2, 0.2));
        tsFeetD->setScale(osg::Vec3(0.015, 0.015, 0.015));
        tsFeetG->setScale(osg::Vec3(0.015, 0.015, 0.015));

        tsTouche->setPosition(osg::Vec3(randX, randY, 3.5));
        tsFeetD->setPosition(osg::Vec3(randX-1.0, randY-0.6, 2.7));
        tsFeetG->setPosition(osg::Vec3(randX+1.0, randY-0.6, 2.7));

        tsTouche->addChild(touche);
        tsFeetD->addChild(feetD);
        tsFeetG->addChild(feetG);

		osg::ref_ptr<osg::PositionAttitudeTransform> theTouche = new osg::PositionAttitudeTransform();

		//theTouche->setAttitude(osg::Quat(osg::DegreesToRadians(angle), osg::Vec3(0.0, 0.0, 1.0)));
        //theTouche->setPosition(osg::Vec3(randX, randY, -1.0));
		theTouche->addChild(tsTouche);
		theTouche->addChild(tsFeetD);
		theTouche->addChild(tsFeetG);

        //Path pour les touches
        osg::ref_ptr<osg::AnimationPath> touchePath = new osg::AnimationPath;
        //Définition du mode de bouclage sur le chemin défini
        touchePath->setLoopMode(osg::AnimationPath::SWING);

        osg::AnimationPath::ControlPoint p0(osg::Vec3(0, 0, 0));
        osg::AnimationPath::ControlPoint p1(osg::Vec3(0, 10, 0));
        osg::AnimationPath::ControlPoint p2(osg::Vec3(0, 20, 3));
        osg::AnimationPath::ControlPoint p3(osg::Vec3(0, 30, 6));
        osg::AnimationPath::ControlPoint p4(osg::Vec3(0, 40, 9));
        osg::AnimationPath::ControlPoint p5(osg::Vec3(0, 50, 12));
        osg::AnimationPath::ControlPoint p6(osg::Vec3(0, 60, 9));
        osg::AnimationPath::ControlPoint p7(osg::Vec3(0, 70, 6));

        osg::AnimationPath::ControlPoint p8(osg::Vec3(0, 80, 3));
        osg::AnimationPath::ControlPoint p9(osg::Vec3(0, 90, 0));
        osg::AnimationPath::ControlPoint p10(osg::Vec3(0, 100, 0));

        touchePath->insert(0.0f, p0);
        touchePath->insert(0.4f, p1);
        touchePath->insert(0.8f, p2);
        touchePath->insert(1.2f, p3);
        touchePath->insert(1.6f, p4);
        touchePath->insert(2.0f, p5);
        touchePath->insert(2.4f, p6);
        touchePath->insert(2.8f, p7);
        touchePath->insert(3.2f, p8);
        touchePath->insert(3.6f, p9);
        touchePath->insert(4.0f, p10);

        osg::ref_ptr<osg::AnimationPathCallback> apc = new osg::AnimationPathCallback(touchePath.get());
        theTouche->setUpdateCallback(apc.get());

		touches->addChild(theTouche);
    }
    return touches;
}

osg::ref_ptr<osg::Group> creation_troupeau_chikoiseau(int nb_chikoiseau, float taillex, float tailley, std::string filename){

	osg::ref_ptr<osg::Sphere> corpsChikoiseau = new osg::Sphere(osg::Vec3(0.0,0.0,5.0), 1.0);
	osg::ref_ptr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(corpsChikoiseau);
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(shapeDrawable);

	// create a simple material
	osg::ref_ptr<osg::Material> material = new osg::Material();
	material->setEmission(osg::Material::FRONT, osg::Vec4(0.8, 0.8, 0.8, 1.0));

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setDataVariance(osg::Object::DYNAMIC);
    texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);

    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
    texture->setImage(image);

	osg::ref_ptr<osg::Node> aileD = osgDB::readNodeFile("wingD.obj");
	osg::ref_ptr<osg::Node> aileG = osgDB::readNodeFile("wingG.obj");

	osg::ref_ptr<osg::Group> troupeau = new osg::Group;
	for(unsigned int i = 0; i < nb_chikoiseau; ++i){
        //osg::ref_ptr<osg::Image> image = osgDB::readImageFile(tete);
        osg::ref_ptr<osg::StateSet> sphereStateSet = geode->getOrCreateStateSet();
        sphereStateSet->ref();
    	sphereStateSet->setAttribute(material);
        sphereStateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

		int randX = rand()%(int)taillex;
		int randY = rand()%(int)tailley;
		osg::ref_ptr<osg::PositionAttitudeTransform> transformChikoiseau = new osg::PositionAttitudeTransform();
		transformChikoiseau->setPosition(osg::Vec3(randX, randY, 1.0));

		float angle = rand()%360;
		transformChikoiseau->setAttitude(osg::Quat(osg::DegreesToRadians(angle), osg::Vec3(0.0, 0.0, 1.0)));
		transformChikoiseau->addChild(geode);

		osg::ref_ptr<osg::PositionAttitudeTransform> transformAileG = new osg::PositionAttitudeTransform();
        osg::ref_ptr<osg::PositionAttitudeTransform> transformAileD = new osg::PositionAttitudeTransform();

		transformAileG->setPosition(osg::Vec3(randX, randY, 6.0));
        transformAileD->setPosition(osg::Vec3(randX, randY, 6.0));

		transformAileG->setAttitude(osg::Quat(osg::DegreesToRadians(angle+90), osg::Vec3(0.0, 0.0, 1.0)));
		transformAileD->setAttitude(osg::Quat(osg::DegreesToRadians(angle+90), osg::Vec3(0.0, 0.0, 1.0)));

        transformAileG->setScale(osg::Vec3(0.5,0.5,0.5));
        transformAileD->setScale(osg::Vec3(0.5,0.5,0.5));

		osg::ref_ptr<osg::PositionAttitudeTransform> Chikoiseau = new osg::PositionAttitudeTransform();
		osg::ref_ptr<osg::PositionAttitudeTransform> aileDRotate = new osg::PositionAttitudeTransform();
		osg::ref_ptr<osg::PositionAttitudeTransform> aileGRotate = new osg::PositionAttitudeTransform();

		aileDRotate->addChild(aileG);
		aileGRotate->addChild(aileD);

	    aileDRotate->setUpdateCallback(new FlapFlapD);
		aileGRotate->setUpdateCallback(new FlapFlapG);

		transformAileG->addChild(aileGRotate);
		transformAileD->addChild(aileDRotate);

		Chikoiseau->addChild(transformAileG);
		Chikoiseau->addChild(transformAileD);
		Chikoiseau->addChild(transformChikoiseau);
		//Chikoiseau->setUpdateCallback(new MovementChikoiseau);

		troupeau->addChild(Chikoiseau);

	}
	return troupeau;
}

osg::ref_ptr<osg::Group> creation_panneaux(int nb_panneaux, float taillex, float tailley, std::string nomImage){

	osg::ref_ptr<osg::Box> shapePanneau = new osg::Box(osg::Vec3(0.0,0.0,7.0), 0.01, 4.0, 4.0);
	osg::ref_ptr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(shapePanneau);
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(shapeDrawable);

	// create a simple material
	osg::ref_ptr<osg::Material> material = new osg::Material();
	material->setEmission(osg::Material::FRONT, osg::Vec4(0.8, 0.8, 0.8, 1.0));

	// create a texture
	// load image for texture
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile(nomImage);
	if (!image) {
		std::cout << "Couldn't load texture." << std::endl;
		return NULL;
	}
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setDataVariance(osg::Object::DYNAMIC);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
	texture->setImage(image);
    //assign the material and texture to the sphere
    osg::ref_ptr<osg::StateSet> boxStateSet = geode->getOrCreateStateSet();
    boxStateSet->ref();
	boxStateSet->setAttribute(material);
	boxStateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

	osg::ref_ptr<osg::Group> troupeau = new osg::Group;
	for(unsigned int i = 0; i < nb_panneaux; ++i){
		int randX = rand()%(int)taillex;
		int randY = rand()%(int)tailley;

		osg::ref_ptr<osg::PositionAttitudeTransform> transformPanneau = new osg::PositionAttitudeTransform();
		transformPanneau->setPosition(osg::Vec3(randX, randY, 1.0));
		float angle = rand()%360;
		transformPanneau->setAttitude(osg::Quat(osg::DegreesToRadians(0.0), osg::Vec3(1.0, 0.0, 0.0), osg::DegreesToRadians(50.0), osg::Vec3(0.0, 1.0, 0.0), osg::DegreesToRadians(angle), osg::Vec3(0.0, 0.0, 1.0)));
		transformPanneau->addChild(geode);

		troupeau->addChild(transformPanneau);
	}
	return troupeau;
}

void CreationCD(){
    osg::ref_ptr<osg::PositionAttitudeTransform> transformCD;
    osg::ref_ptr<osg::Node> CD;

 	CD = osgDB::readNodeFile("cd.3ds");

 	transformCD = new osg::PositionAttitudeTransform;
 	transformCD->setPosition(osg::Vec3(0,0,0));
    transformCD->setScale(osg::Vec3(1.0, 1.0, 1.0));
 	transformCD->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);

 	transformCD->addChild(CD);

    //Path pour les touches
    osg::ref_ptr<osg::AnimationPath> cdPath = new osg::AnimationPath;
    //Définition du mode de bouclage sur le chemin définiosg::ref_ptr<osg::Node> feetG = osgDB::readNodeFile("feetG.obj");
    osg::ref_ptr<osg::Node> touche = osgDB::readNodeFile("key.3ds");
    cdPath->setLoopMode(osg::AnimationPath::LOOP);

    float height = 0.0;

    osg::AnimationPath::ControlPoint p0rot(osg::Vec3(100, 0.0, height), osg::Quat(osg::DegreesToRadians(90.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p1rot(osg::Vec3(100, 800, height), osg::Quat(osg::DegreesToRadians(90.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p2rot(osg::Vec3(200, 900, height), osg::Quat(osg::DegreesToRadians(00.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p3rot(osg::Vec3(300, 800, height), osg::Quat(osg::DegreesToRadians(-90.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p4rot(osg::Vec3(300, 200, height), osg::Quat(osg::DegreesToRadians(-90.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p5rot(osg::Vec3(400, 100, height), osg::Quat(osg::DegreesToRadians(00.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p6rot(osg::Vec3(500, 200, height), osg::Quat(osg::DegreesToRadians(90.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p7rot(osg::Vec3(500, 800, height), osg::Quat(osg::DegreesToRadians(90.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p8rot(osg::Vec3(600, 900, height), osg::Quat(osg::DegreesToRadians(00.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p9rot(osg::Vec3(700, 800, height), osg::Quat(osg::DegreesToRadians(-90.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p10rot(osg::Vec3(700, 200, height), osg::Quat(osg::DegreesToRadians(-90.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p11rot(osg::Vec3(800, 100, height), osg::Quat(osg::DegreesToRadians(00.0), osg::Vec3(0.0, 0.0, 1.0)));
    osg::AnimationPath::ControlPoint p12rot(osg::Vec3(900, 0.0, height), osg::Quat(osg::DegreesToRadians(90.0), osg::Vec3(0.0, 0.0, 1.0)));

    cdPath->insert(0.0f, p0rot);
    cdPath->insert(8.0f, p1rot);
    cdPath->insert(10.0f, p2rot);
    cdPath->insert(12.0f, p3rot);
    cdPath->insert(20.0f, p4rot);
    cdPath->insert(22.0f, p5rot);
    cdPath->insert(24.0f, p6rot);
    cdPath->insert(32.0f, p7rot);
    cdPath->insert(34.0f, p8rot);
    cdPath->insert(36.0f, p9rot);
    cdPath->insert(44.0f, p10rot);
    cdPath->insert(46.0f, p11rot);
    cdPath->insert(48.0f, p12rot);

    osg::ref_ptr<osg::AnimationPathCallback> apc = new osg::AnimationPathCallback(cdPath.get());
    transformCD->setUpdateCallback(apc.get());

 	scene->addChild(transformCD);
}

void Creationfeet(){

 	feet = osgDB::readNodeFile("feet.3ds");

 	transformFeet = new osg::PositionAttitudeTransform;
 	transformFeet->setUpdateCallback(new Rotation);
 	transformFeet->setPosition(osg::Vec3(0,0,0));
 	//transformFeet->setScale(osg::Vec3(0.01,0.01,0.01));
 	//transformFeet->setScale(osg::Vec3(1000,1000,1000));
 	transformFeet->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);
 	transformFeet->addChild(feet);

 	scene->addChild(transformFeet);
}

void CreationWalls(){

	osg::ref_ptr<osg::Geometry> quad1 = osg::createTexturedQuadGeometry(
	osg::Vec3(0.0, 0.0, 0.0), // Coin de départ
	osg::Vec3(1000.0, 0.0, 0.0),  // largeur
	osg::Vec3(0.0, 0.0, 40.0),  // hauteur
	0.0, 0.0, 1.0, 1.0); 		// Coordonnées de texture gauche/bas/droit/haut
								// Si vous mettez 4.0 à la place de 1.0,
								// la texture sera répétée 4 fois
	osg::ref_ptr<osg::Geometry> quad2 = osg::createTexturedQuadGeometry(
	osg::Vec3(0.0, 0.0, 0.0), // Coin de départ
	osg::Vec3(0.0, 1000.0, 0.0),  // largeur
	osg::Vec3(0.0, 0.0, 40.0),  // hauteur
	0.0, 0.0, 1.0, 1.0); 		// Coordonnées de texture gauche/bas/droit/haut
								// Si vous mettez 4.0 à la place de 1.0,
								// la texture sera répétée 4 fois
	osg::ref_ptr<osg::Geometry> quad3 = osg::createTexturedQuadGeometry(
	osg::Vec3(0.0, 1000.0, 0.0), // Coin de départ
	osg::Vec3(1000.0, 0.0, 0.0),  // largeur
	osg::Vec3(0.0, 0.0, 40.0),  // hauteur
	0.0, 0.0, 1.0, 1.0); 		// Coordonnées de texture gauche/bas/droit/haut
								// Si vous mettez 4.0 à la place de 1.0,
								// la texture sera répétée 4 fois
	osg::ref_ptr<osg::Geometry> quad4 = osg::createTexturedQuadGeometry(
	osg::Vec3(1000.0, 0.0, 0.0), // Coin de départ
	osg::Vec3(0.0, 1000.0, 0.0),  // largeur
	osg::Vec3(0.0, 0.0, 40.0),  // hauteur
	0.0, 0.0, 1.0, 1.0); 		// Coordonnées de texture gauche/bas/droit/haut
								// Si vous mettez 4.0 à la place de 1.0,
								// la texture sera répétée 4 fois

	// create a simple material
	osg::ref_ptr<osg::Material> material = new osg::Material();
	material->setEmission(osg::Material::FRONT, osg::Vec4(0.0, 0.0, 0.0, 1.0));

	osg::ref_ptr<osg::Geode> geode1 = new osg::Geode;
	osg::ref_ptr<osg::Geode> geode2 = new osg::Geode;
	osg::ref_ptr<osg::Geode> geode3 = new osg::Geode;
	osg::ref_ptr<osg::Geode> geode4 = new osg::Geode;

	osg::ref_ptr<osg::StateSet> boxStateSet = geode1->getOrCreateStateSet();
    boxStateSet->ref();
	boxStateSet->setAttribute(material);

	boxStateSet = geode2->getOrCreateStateSet();
    boxStateSet->ref();
	boxStateSet->setAttribute(material);

	boxStateSet = geode3->getOrCreateStateSet();
    boxStateSet->ref();
	boxStateSet->setAttribute(material);

	boxStateSet = geode4->getOrCreateStateSet();
    boxStateSet->ref();
	boxStateSet->setAttribute(material);

	geode1->addDrawable(quad1);
	geode2->addDrawable(quad2);
	geode3->addDrawable(quad3);
	geode4->addDrawable(quad4);
	scene->addChild(geode1);
	scene->addChild(geode2);
	scene->addChild(geode3);
	scene->addChild(geode4);
}

osg::ref_ptr<osg::Group> creation_numberOne(int nb_number, float startX, float startY, float length, std::string filename){
    osg::ref_ptr<osg::Group> numbersOne = new osg::Group;
    osg::ref_ptr<osg::Node> numberOne = osgDB::readNodeFile(filename);

    float xx = startX;
    float yy = startY;
    float zz = 60.0f;
    float scale = 1.0f;

    for(unsigned int i=0; i <= nb_number; ++i){
        osg::ref_ptr<osg::PositionAttitudeTransform> patOne = new osg::PositionAttitudeTransform();
        /*patOne->setPosition(osg::Vec3(xx, yy, zz));
        patOne->setScale(osg::Vec3(10.0, 10.0, 10.0));
        patOne->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);*/
        patOne->addChild(numberOne);

        //Path pour les touches
        osg::ref_ptr<osg::AnimationPath> nbPath = new osg::AnimationPath;
        //Définition du mode de bouclage sur le chemin définiosg::ref_ptr<osg::Node> feetG = osgDB::readNodeFile("feetG.obj");
        nbPath->setLoopMode(osg::AnimationPath::LOOP);

        osg::AnimationPath::ControlPoint p0rot(osg::Vec3(xx,yy,zz), osg::Quat(osg::DegreesToRadians(-45.0), osg::Vec3(0.0, 1.0, 0.0)), osg::Vec3(scale, scale, scale));
        osg::AnimationPath::ControlPoint p1rot(osg::Vec3(xx,yy+length, zz), osg::Quat(osg::DegreesToRadians(-45.0), osg::Vec3(0.0, 1.0, 0.0)), osg::Vec3(scale, scale, scale));
        osg::AnimationPath::ControlPoint p2rot(osg::Vec3(xx,yy+length, zz), osg::Quat(osg::DegreesToRadians(180.0), osg::Vec3(0.0, 0.0, 1.0)), osg::Vec3(scale, scale, scale));
        osg::AnimationPath::ControlPoint p3rot(osg::Vec3(xx,yy,zz), osg::Quat(osg::DegreesToRadians(45.0), osg::Vec3(0.0, 1.0, 0.0)), osg::Vec3(scale, scale, scale));
        osg::AnimationPath::ControlPoint p4rot(osg::Vec3(xx,yy,zz), osg::Quat(osg::DegreesToRadians(180.0), osg::Vec3(0.0, 0.0, 1.0)), osg::Vec3(scale, scale, scale));

        nbPath->insert(0.0f, p0rot);
        nbPath->insert(10.0f, p1rot);
        nbPath->insert(12.0f, p2rot);
        nbPath->insert(22.0f, p3rot);
        nbPath->insert(24.0f, p4rot);

        osg::ref_ptr<osg::AnimationPathCallback> apc = new osg::AnimationPathCallback(nbPath.get());
        patOne->setUpdateCallback(apc.get());
        numbersOne->addChild(patOne);

        xx+=20;
        }
    return numbersOne;
}

int main(void){
    srand(time(NULL));
	osg::DisplaySettings::instance()->setNumMultiSamples( 4 );
	viewer.setUpViewInWindow( 100, 50, 1920, 1080 );
	viewer.getCamera()->setClearColor( osg::Vec4( 0.0,0.0,0.0,1) );
	viewer.addEventHandler(new osgViewer::StatsHandler);
	manip = new osgGA::DriveManipulator();
	viewer.setCameraManipulator(manip.get());
	scene = new osg::Group;
	root = new osg::Group;

	osg::ref_ptr<osg::LightSource> lumiere = new osg::LightSource;
	lumiere->getLight()->setLightNum(1); // GL_LIGHT1
	lumiere->getLight()->setPosition(osg::Vec4(50, 50, 10, 0)); // 0 = directionnel
	lumiere->getLight()->setAmbient(osg::Vec4(0.3, 0.3, 0.3, 1.0));
	lumiere->getLight()->setDiffuse(osg::Vec4(0.5, 0.5, 0.5, 1.0));
	lumiere->getLight()->setSpecular(osg::Vec4(1.0, 1.0, 1.0, 1.0));
	osg::ref_ptr<osg::StateSet> state = scene->getOrCreateStateSet();
	state->setMode( GL_LIGHT0, osg::StateAttribute::OFF );
	state->setMode( GL_LIGHT1, osg::StateAttribute::ON );
	root->addChild(lumiere);

	root->addChild(scene);
	CreateSol();
    //Creationfeet();
    CreationCD();
    CreationWalls();
	scene->addChild(geodeSol);
    //scene->addChild(creation_memoryleak(0, fieldX, fieldY, "du_coup.jpg"));
    //scene->addChild(creation_memoryleak(10, fieldX, fieldY, "01.jpg"));
	scene->addChild(creation_troupeau_chikoiseau(25, fieldX, fieldY,"remy.jpg"));
	scene->addChild(creation_troupeau_chikoiseau(25, fieldX, fieldY,"raffin.jpg"));
	scene->addChild(creation_troupeau_chikoiseau(25, fieldX, fieldY,"thon.jpeg"));
	scene->addChild(creation_troupeau_chikoiseau(25, fieldX, fieldY,"triboulet.jpg"));
	scene->addChild(creation_troupeau_chikoiseau(25, fieldX, fieldY,"tibo.jpg"));
    scene->addChild(creation_troupeau_touches(100, fieldX, fieldY));
    scene->addChild(creation_panneaux(80, fieldX, fieldY, "stravingo.jpeg"));
    scene->addChild(creation_panneaux(80, fieldX, fieldY, "doge.jpeg"));
    scene->addChild(creation_panneaux(20, fieldX, fieldY, "nvidia.png"));
    scene->addChild(creation_lampadaires(100, fieldX, fieldY));
    scene->addChild(creation_procs(30, fieldX, fieldY));
    //scene->addChild(creation_ventirads(45, fieldX, fieldY));
    scene->addChild(creation_condens(45, fieldX, fieldY));
    scene->addChild(creation_rams(250, fieldX, fieldY));
    for (float i = 0, y = 0; i < 300; i = i + 20.0, ++y) {
        scene->addChild(creation_numberOne(50, y, i, fieldX, "1.3ds"));
        scene->addChild(creation_numberOne(50, y + 10.0, i, fieldX, "0.3ds"));
    }
    //CreationCiel();
	//scene->addChild(geodeCiel);
	//manip->setNode(geodeSol);
	//manip->setHeight(1.5);
	viewer.setSceneData(root);


    /*patSpeed = new osg::PositionAttitudeTransform;
    patSpeed->setUpdateCallback(new RefreshSpeed);
    scene->addChild(patSpeed);*/

    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("PIGS_WORLD_by_ANTICEPTIK_KAOTEK2.ogg"))
        return -1;

    sf::Sound sound;
    sound.setBuffer(buffer);
    sound.play();
    sound.setLoop(true);
    sound.setVolume(100);

    // sf::SoundBuffer buffer_;
    // if (!buffer_.loadFromFile("No_No_No_Cat.ogg"))
    //     return -1;
    //
    // sf::Sound sound_;
    // sound_.setBuffer(buffer_);
    // sound_.play();
    // sound_.setLoop(true);
    // sound_.setVolume(100);

    viewer.setRunFrameScheme(osgViewer::ViewerBase::ON_DEMAND);
    viewer.setRunFrameScheme(osgViewer::ViewerBase::CONTINUOUS);
    viewer.setRunMaxFrameRate(60.0);

	osg::ref_ptr<GestionEvenements> gestionnaire = new GestionEvenements();
	viewer.addEventHandler(gestionnaire.get());

	return viewer.run();
}
