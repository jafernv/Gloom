//---------------------------------------------------------------------------
// CompositorListener.cpp
//---------------------------------------------------------------------------

/**
@file CompositorListener.cpp

Contiene la implementacion del compositor listener, este se usa para pasar parametros a los compositor (y los shader aplicados a estos)

@author Antonio Jesus Narvaez
@date Abril, 2013
*/

#include "CompositorListener.h"

#include <map>
#ifndef __Graphics_COMPOSITORLISTENER_H
#define __Graphics_COMPOSITORLISTENER_H

#include <OgreCompositorInstance.h>
#include <OgreGpuProgramParams.h>
#include <OgreCompositorManager.h>
#include <OgreViewport.h>
#include <OgreCamera.h>
#include <OgreCompositorChain.h>
#include <OgreCompositor.h>

#include "Graphics\Server.h"
#include "Graphics\Scene.h"
#include "Graphics\Camera.h"

namespace Graphics 
{
	
	
	CCompositorListener::CCompositorListener ()
		{
			_compositorManager = Ogre::CompositorManager::getSingletonPtr();
			_cameraViewport = Graphics::CServer::getSingletonPtr()->getActiveScene()->getCamera()->getOgreCamera()->getViewport();
		}

		CCompositorListener::~CCompositorListener(){};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		void CCompositorListener::inputCompositor(const std::string &nameVar, float value){
			
			std::pair<std::string, float> elem(nameVar,value);

			_compositorVariables[nameVar] = value;

		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void CCompositorListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
		{
				necesaryOperations(pass_id, mat);
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void CCompositorListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
		{
				necesaryOperations(pass_id, mat);
		}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void CCompositorListener::necesaryOperations(int pass_id, Ogre::MaterialPtr &mat){
			mat->load();

			
			if(!mat->getTechnique(0)->getPass(0)->hasFragmentProgram())
				return;
			//const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
			//*
			Ogre::GpuProgramParametersSharedPtr fpParams = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();

			for (TVariables::const_iterator it = _compositorVariables.begin(); it!= _compositorVariables.end(); ++it){
					//we must check if the variable we are changing is defined
					if(fpParams->getConstantDefinitions().map.find(it->first)!=fpParams->getConstantDefinitions().map.end())
					fpParams->setNamedConstant(it->first, it->second);
			}
			
			/*/

			Ogre::GpuProgramParametersSharedPtr fpParams;
			Ogre::Technique *tech= mat->getTechnique(0);
			if( tech  != 0){
				Ogre::Pass *pass = tech->getPass(0);
				if(( pass != 0) && (pass->hasFragmentProgram())){
					fpParams = pass->getFragmentProgramParameters();
					if(!fpParams.isNull()){
					
						for (TVariables::const_iterator it = _compositorVariables.begin(); it!= _compositorVariables.end(); ++it)
						{
							//we must check if the variable we are changing is defined
							if(fpParams->getConstantDefinitions().map.find(it->first)!=fpParams->getConstantDefinitions().map.end())
							fpParams->setNamedConstant(it->first, it->second);
						}
					}
				}
			}
			/* */
			

			
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif // __Graphics_CompositorListener_H