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
#ifndef MODELDATA_MESH_H
#define MODELDATA_MESH_H

#include <vector>
#include "MeshPart.h"
#include "Attributes.h"
#include "../json/BaseJSONWriter.h"

namespace fbxconv {
namespace modeldata {
	/** A mesh is responsable for freeing all parts and vertices it contains. */
	struct Mesh : public json::ConstSerializable {
		/** the attributes the vertices in this mesh describe */
		Attributes attributes;
		/** the size (in number of floats) of each vertex */
		unsigned int vertexSize;
		/** the vertices that this mesh contains */
		std::vector<float> vertices;
		/** hash lookup table for faster duplicate vertex checking */
		std::vector<unsigned int> hashes;
		/** the indexed parts of this mesh */
		std::vector<MeshPart *> parts;

		/** ctor */
		Mesh() : attributes(0), vertexSize(0) {}
        
        /**
         Extension by XuSheng
         */
        int attributeSize_old=0;
        int attributeSize_new=0;
        int samecount=0;
        
		/** copy constructor */
		Mesh(const Mesh &copyFrom) {
			attributes = copyFrom.attributes;
			vertexSize = copyFrom.vertexSize;
			vertices.insert(vertices.end(), copyFrom.vertices.begin(), copyFrom.vertices.end());
			for (std::vector<MeshPart *>::const_iterator itr = copyFrom.parts.begin(); itr != copyFrom.parts.end(); ++itr)
				parts.push_back(new MeshPart(**itr));
		}

		~Mesh() {
			clear();
		}
        
        /**
         Extension by XuSheng
         */
        void clearVertices(){
            vertices.resize(10);
        }

        /**
         Extension by XuSheng
         */
        void addACellData(int loc, std::vector<float> &finalVector){
            for(int i=0;i<attributeSize_old;i++){
                if(i==3 || i==4 || i==5);
                else
                    finalVector.push_back(vertices[loc+i]);
            }
        }
        
        /**
         Extension by XuSheng
         */
        void ADD_deleteNormal(){
            std::vector<float> finalVector;
            // int cellSpan=16;
            attributeSize_old=attributes.size();
            attributes.hasNormal(false);
            attributeSize_new=attributes.size();
            
            for(int i=0;i<vertices.size();i+=attributeSize_old){
                if(i==0) addACellData(0, finalVector);
                else{
                    int loc=getSameVexterLoc(i, finalVector);
                    if(loc>=0){
                        samecount++;
                    }else{
                        addACellData(i, finalVector);
                    }
                }
                
            }
            printf("-----same count : %d, old count :  %lu, left vertex : %lu\n",
                   samecount,vertices.size()/attributeSize_old,vertices.size()/attributeSize_old-samecount);
            
            printf("-----the old attributes size: %d  , the new attributes size:%d .\n",
                   attributeSize_old,attributeSize_new);
            
            deleteSameIndex(finalVector);
            
            vertices.clear();
            vertices=finalVector;
            
            /**
             the vertexSize control the output size of the vertex
             **/
            vertexSize=attributeSize_new;
            
            
        }
        
        /**
         Extension by XuSheng
         */
        void deleteSameIndex(std::vector<float> &finalVector){
            for (std::vector<MeshPart *>::const_iterator itr = parts.begin(); itr != parts.end(); ++itr){
                for(int i=0;i<(*itr)->indices.size();i++ ){
                    int index=(*itr)->indices[i];
                    index*=attributeSize_old;
                    int loc=getSameVexterLoc(index, finalVector);
                    (*itr)->indices[i]=loc/attributeSize_new;
                }
            }
            
        }
        
        /**
         Extension by XuSheng
         */
        int getSameVexterLoc(int loc,std::vector<float> &finalVector){
            for(int i=0;i<finalVector.size();i+=attributeSize_new ){
                int j=0;
                for( j=0;j<attributeSize_new;j++){
                    if(finalVector[i+j]==vertices[loc+(j>2?j+3:j)]);
                    else break;
                }
                if(j==attributeSize_new) return i;
                
            }
            return -1;
        }

		void clear() {
			vertices.clear();
			hashes.clear();
			attributes = vertexSize = 0;
			for (std::vector<MeshPart *>::iterator itr = parts.begin(); itr != parts.end(); ++itr)
				delete (*itr);
			parts.clear();
		}

		inline unsigned int indexCount() {
			unsigned int result = 0;
			for (std::vector<MeshPart *>::const_iterator itr = parts.begin(); itr != parts.end(); ++itr)
				result += (unsigned int)(*itr)->indices.size();
			return result;
		}

		inline unsigned int vertexCount() {
			return vertices.size() / vertexSize;
		}

		inline unsigned int add(const float *vertex) {
			const unsigned int hash = calcHash(vertex, vertexSize);
			const unsigned int n = (unsigned int)hashes.size();
			for (unsigned int i = 0; i < n; i++)
				if ((hashes[i] == hash) && compare(&vertices[i*vertexSize], vertex, vertexSize))
					return i;
			hashes.push_back(hash);
			vertices.insert(vertices.end(), &vertex[0], &vertex[vertexSize]);
			return (unsigned int)hashes.size() - 1;
		}

		inline unsigned int calcHash(const float *vertex, const unsigned int size) {
			unsigned int result = 0;
			for (unsigned int i = 0; i < size; i++)
				result += ((*((unsigned int *)&vertex[i])) & 0xffffff00) >> 8;
			return result;
		}

		inline bool compare(const float* lhs, const float* rhs, const unsigned int &n) {
			for (unsigned int i = 0; i < n; i++)
				if ((*(unsigned int*)&lhs[i] != *(unsigned int*)&rhs[i]) && lhs[i] != rhs[i])
					return false;
			return true;
		}

		virtual void serialize(json::BaseJSONWriter &writer) const;
	};
}
}

#endif //MODELDATA_MESH_H
