#pragma once
#include "Prerequisites.h"
//#include "ECS\Component.h"

class DeviceContext;

class
  MeshComponent /*: public Component*/ {
public:
  MeshComponent() : m_numVertex(0), m_numIndex(0)/*, Component(ComponentType::MESH)*/ {}

  virtual
  ~MeshComponent() = default;

  /**
   * @brief Actualiza el actor.
   * @param deltaTime El tiempo transcurrido desde la última actualización.
   * @param deviceContext Contexto del dispositivo para operaciones gráficas.
   */
  void
  update(float deltaTime) /*override*/ {}

  /**
   * @brief Renderiza el actor.
   * @param deviceContext Contexto del dispositivo para operaciones gráficas.
   */
  void
  render(DeviceContext& deviceContext) /*override*/ {}
public:
  std::string m_name;
  std::vector<SimpleVertex> m_vertex;
  std::vector<unsigned int> m_index;
  int m_numVertex;
  int m_numIndex;
};
