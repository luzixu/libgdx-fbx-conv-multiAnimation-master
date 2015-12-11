/*******************************************************************************
 * Copyright 2011 See AUTHORS file.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
/** @author Xoppa */
#ifdef _MSC_VER 
#pragma once
#endif
#ifndef MODELDATA_MODEL_H
#define MODELDATA_MODEL_H

#include <vector>
#include <string>
#include "Animation.h"
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "../json/BaseJSONWriter.h"
#include "../Settings.h"

namespace fbxconv {
namespace modeldata {
	const short VERSION_HI = 0;
	const short VERSION_LO = 1;

	/** A model is responsable for freeing all animations, materials, meshes and nodes it contains */
	struct Model : public json::ConstSerializable {
		short version[2];
		std::string id;
		std::vector<Animation *> animations;
		std::vector<Material *> materials;
		std::vector<Mesh *> meshes;
		std::vector<Node *> nodes;

		Model() { version[0] = VERSION_HI; version[1] = VERSION_LO; }

		Model(const Model &copyFrom) {
			version[0] = copyFrom.version[0];
			version[1] = copyFrom.version[1];
			id = copyFrom.id;
			for (std::vector<Animation *>::const_iterator itr = copyFrom.animations.begin(); itr != copyFrom.animations.end(); ++itr)
				animations.push_back(new Animation(**itr));
			for (std::vector<Material *>::const_iterator itr = copyFrom.materials.begin(); itr != copyFrom.materials.end(); ++itr)
				materials.push_back(new Material(**itr));
			for (std::vector<Mesh *>::const_iterator itr = copyFrom.meshes.begin(); itr != copyFrom.meshes.end(); ++itr)
				meshes.push_back(new Mesh(**itr));
			for (std::vector<Node *>::const_iterator itr = copyFrom.nodes.begin(); itr != copyFrom.nodes.end(); ++itr)
				nodes.push_back(new Node(**itr));
		}

		~Model() {
			clear();
		}

		void clear() {
			for (std::vector<Animation *>::iterator itr = animations.begin(); itr != animations.end(); ++itr)
				delete *itr;
			animations.clear();
			for (std::vector<Material *>::iterator itr = materials.begin(); itr != materials.end(); ++itr)
				delete *itr;
			materials.clear();
			for (std::vector<Mesh *>::iterator itr = meshes.begin(); itr != meshes.end(); ++itr)
				delete *itr;
			meshes.clear();
			for (std::vector<Node *>::iterator itr = nodes.begin(); itr != nodes.end(); ++itr)
				delete *itr;
			nodes.clear();
		}
        
        /**
         Extendsion by XuSheng
         */
        void clearMeshVertix(){
            for (std::vector<Mesh *>::iterator itr = meshes.begin(); itr != meshes.end(); ++itr)
                (*itr)->clearVertices();
        }
        
        /**
         Extendsion by XuSheng
         */
        void deleteNormal(const Settings * settings){
            if(settings->deleteNormal){
                for (std::vector<Mesh *>::iterator itr = meshes.begin(); itr != meshes.end(); ++itr)
                    (*itr)->ADD_deleteNormal();
            }
        }
        
        /**
         Extendsion by XuSheng
         */
        void breakAnimation(const Settings * settings){
            if(settings->isExtractAnimation)
                breakAnimation_isExtractAnimation(settings);
            else
                breakAnimation_new(settings);
        }
        
        
        /**
         Extendsion by XuSheng
         */
        void breakAnimation_isExtractAnimation(const Settings * settings){
            
            int breakSize=settings->breakAnimationIds.size();
            for(int i=0;i<settings->animation_span_loc.size();i++){
                if(i%2==0)
                    printf("-----animation id : %s\n",settings->breakAnimationIds.at(i/2).c_str());
                printf("-----span loc %d  : %d \n",i,settings->animation_span_loc.at(i));
                
            }
            printf("-----breakSize      : %d \n",breakSize);
            
            std::vector<Animation *> breakAnimations;
            Animation *oAnimaiton=animations.at(0);
            float KeyTimeCell=41.666667;
            /**
             create break animation
             **/
            for(int i=0;i<breakSize;i++){
                Animation *newAnimation = new Animation();
                newAnimation->id=settings->breakAnimationIds.at(i);
                breakAnimations.push_back(newAnimation);
            }
            
            std::vector<NodeAnimation *> nodes=oAnimaiton->nodeAnimations;
            
            for(int i=0;i<nodes.size();i++){
                NodeAnimation * nodeAnimation=nodes.at(i);
                
                for(int t=0;t<breakAnimations.size();t++){
                    NodeAnimation * newNodeAnimation=new NodeAnimation();
                    newNodeAnimation->node=nodeAnimation->node;
                    breakAnimations.at(t)->nodeAnimations.push_back(newNodeAnimation);
                }
                
                int sT,eT;
                int loc1,loc2,loc_k1,loc_k2;
                
                for(int j=0;j<breakSize;j++){
                    loc1=settings->animation_span_loc.at(j*2+0);
                    loc2=settings->animation_span_loc.at(j*2+1);
                    int length=nodeAnimation->keyframes.size();
                    printf("node id= %s size= %d \n",nodeAnimation->node->id.c_str(),length);
                    Keyframe * k1,*k2,* k3,* newKey;
                    if(nodeAnimation->keyframes.size()>0) k1=nodeAnimation->keyframes.at(0);
                    bool content=false;
                    for(int t=1;t<length;t++){
                        k2=nodeAnimation->keyframes.at(t);
                        loc_k1=k1->time/KeyTimeCell;
                        loc_k2=k2->time/KeyTimeCell;
                        //                            printf("k time=%f \n",k1->time);
                        // if(j==5) printf("loc_k1=%d loc_k2=%d ",loc_k1,loc_k2);
                        if(loc_k1<loc1 ){
                            if(loc_k2>loc1 && loc_k2<loc2){
                                newKey=new Keyframe(*k1);
                                newKey->time=0;
                                breakAnimations.at(j)->nodeAnimations.at(i)->keyframes.push_back(newKey);
                                content=true;
                            }else if(loc_k2>=loc2){
                                newKey=new Keyframe(*k1);
                                newKey->time=0;
                                breakAnimations.at(j)->nodeAnimations.at(i)->keyframes.push_back(newKey);
                                newKey=new Keyframe(*k1);
                                newKey->time=(loc2-loc1-1)*KeyTimeCell;
                                breakAnimations.at(j)->nodeAnimations.at(i)->keyframes.push_back(newKey);
                                content=true;
                            }
                        }else if(loc_k1>=loc1 && loc_k1<loc2){
                            if(loc_k2<=loc2){
                                newKey=new Keyframe(*k1);
                                newKey->time=(loc_k1-loc1)*KeyTimeCell;
                                breakAnimations.at(j)->nodeAnimations.at(i)->keyframes.push_back(newKey);
                                content=true;
                            }else{
                                newKey=new Keyframe(*k1);
                                newKey->time=0;
                                breakAnimations.at(j)->nodeAnimations.at(i)->keyframes.push_back(newKey);
                                newKey=new Keyframe(*k1);
                                newKey->time=(loc2-loc1-1)*KeyTimeCell;
                                breakAnimations.at(j)->nodeAnimations.at(i)->keyframes.push_back(newKey);
                                content=true;
                            }
                        }
                        k1=k2;
                    }
                }
                
            }
            
            animations.clear();
            animations=breakAnimations;
            
            printf("-----breakAnimaiton over...\n");
            
        }


        /**
         Extendsion by XuSheng
         */
        void breakAnimation_new(const Settings * settings){
            
            //        char * animation_ids[] ={"walk","hold","fall down","run","fall forward","fall back"};
            //        int  animation_span_loc [][2]={{0,40},{44,167},{172,191},{196,212},{217,241},{246,271}};
            //        int breakSize=((sizeof animation_span_loc)/sizeof(int))/2;
            
            
            int breakSize=settings->breakAnimationIds.size();
            for(int i=0;i<settings->animation_span_loc.size();i++){
                if(i%2==0)
                    printf("-----animation id : %s\n",settings->breakAnimationIds.at(i/2).c_str());
                printf("-----span loc %d  : %d \n",i,settings->animation_span_loc.at(i));
                
            }
            printf("-----breakSize      : %d \n",breakSize);
            
            
            
            
            std::vector<Animation *> breakAnimations;
            
            Animation *oAnimaiton=animations.at(0);
            //        float KeyTimeCell=0.0416666666666667*1000;
            float KeyTimeCell=41.666667;
            /**
             create break animation
             **/
            for(int i=0;i<breakSize;i++){
                Animation *newAnimation = new Animation();
                //            newAnimation->id=animation_ids[i];
                newAnimation->id=settings->breakAnimationIds.at(i);
                breakAnimations.push_back(newAnimation);
            }
            
            
            
            std::vector<NodeAnimation *> nodes=oAnimaiton->nodeAnimations;
            
            for(int i=0;i<nodes.size();i++){
                NodeAnimation * nodeAnimation=nodes.at(i);
                
                for(int t=0;t<breakAnimations.size();t++){
                    NodeAnimation * newNodeAnimation=new NodeAnimation();
                    newNodeAnimation->node=nodeAnimation->node;
                    breakAnimations.at(t)->nodeAnimations.push_back(newNodeAnimation);
                }
                int old_loc=0;
                for(int j=0;j<nodeAnimation->keyframes.size();j++){
                    int loc=0;
                    Keyframe * keyframe=nodeAnimation->keyframes.at(j);
                    while(keyframe->time>settings->animation_span_loc.at(loc*2+0)*KeyTimeCell &&
                          keyframe->time>settings->animation_span_loc.at(loc*2+1)*KeyTimeCell ){
                        loc++;
                        if(loc>=breakSize) break;
                    }
                    if(loc<breakSize){
                        //                    printf("-----keytime      : %f, loc=:    %d, spanLoctime:  %f  , span= %d \n",
                        //                           keyframe->time,loc,settings->animation_span_loc.at(loc*2+0)*KeyTimeCell,settings->animation_span_loc.at(loc*2+0));
                        //
                        if(keyframe->time>=settings->animation_span_loc.at(loc*2+0)*KeyTimeCell ){
                            keyframe->time-=settings->animation_span_loc.at(loc*2+0)*KeyTimeCell ;
                            breakAnimations.at(loc)->nodeAnimations.at(i)->keyframes.push_back(new Keyframe(*keyframe));
                        }
                    }
                    
                }
            }
            
            animations.clear();
            animations=breakAnimations;
            
            printf("-----breakAnimaiton over...\n");
            
        }


		Node *getNode(const char *id) const {
			for (std::vector<Node *>::const_iterator itr = nodes.begin(); itr != nodes.end(); ++itr) {
				if ((*itr)->id.compare(id)==0)
					return *itr;
				Node *cnode = (*itr)->getChild(id);
				if (cnode != NULL)
					return cnode;
			}
			return NULL;
		}

		Material *getMaterial(const char *id) const {
			for (std::vector<Material *>::const_iterator itr = materials.begin(); itr != materials.end(); ++itr)
				if ((*itr)->id.compare(id)==0)
					return *itr;
			return NULL;
		}

		size_t getTotalNodeCount() const {
			size_t result = nodes.size();
			for (std::vector<Node*>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
				result += (*it)->getTotalNodeCount();
			return result;
		}

		size_t getTotalNodePartCount() const {
			size_t result = 0;
			for (std::vector<Node*>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
				result += (*it)->getTotalNodePartCount();
			return result;
		}

		size_t getMeshpartCount() const {
			size_t result = 0;
			for (std::vector<Mesh *>::const_iterator it = meshes.begin(); it != meshes.end(); ++it)
				result += (*it)->parts.size();
			return result;
		}

		size_t getTotalVertexCount() const {
			size_t result = 0;
			for (std::vector<Mesh *>::const_iterator it = meshes.begin(); it != meshes.end(); ++it)
				result += (*it)->vertexCount();
			return result;
		}

		size_t getTotalIndexCount() const {
			size_t result = 0;
			for (std::vector<Mesh *>::const_iterator it = meshes.begin(); it != meshes.end(); ++it)
				result += (*it)->indexCount();
			return result;
		}

		size_t getTotalTextureCount() const {
			size_t result = 0;
			for (std::vector<Material*>::const_iterator it = materials.begin(); it != materials.end(); ++it)
				result += (*it)->textures.size();
			return result;
		}

		virtual void serialize(json::BaseJSONWriter &writer) const;
	};
}
}

#endif
