class MockBME280 : public ISensor {
public:
    bool init() override { return true; }
    float readData(DataType type) override {
        switch(type) {
            case DataType::TEMPERATURE: return 25.0f;
            case DataType::PRESSURE: return 1013.25f;
            case DataType::HUMIDITY: return 50.0f;
            default: return 0.0f;
        }
    }
    // Implement other interface methods...
};