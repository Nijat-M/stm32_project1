/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DHT11_PORT GPIOB
#define DHT11_PIN GPIO_PIN_9
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// 1. 微秒延时函数
void microDelay(uint16_t delay)
{
__HAL_TIM_SET_COUNTER(&htim1, 0);
while (__HAL_TIM_GET_COUNTER(&htim1) < delay);
}

// 2. 设置引脚为输出模式
void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = GPIO_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

// 3. 设置引脚为输入模式
void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = GPIO_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = GPIO_NOPULL; // 假设外部已有上拉电阻
HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

// 4. DHT11 开始信号
uint8_t DHT11_Start(void)
{
uint8_t Response = 0;
Set_Pin_Output(DHT11_PORT, DHT11_PIN);  // 设置为输出
HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, 0); // 拉低
HAL_Delay(20); // 至少18ms
HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, 1); // 拉高
microDelay(30); // 等待 30us
Set_Pin_Input(DHT11_PORT, DHT11_PIN); // 设置为输入

// 等待传感器响应
uint16_t timeout = 0;
while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))
{
  timeout++;
  if(timeout > 1000) return 0; // 超时
}
timeout = 0;
while(!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))
{
  timeout++;
  if(timeout > 1000) return 0; // 超时
}
timeout = 0;
while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))
{
  timeout++;
  if(timeout > 1000) return 0; // 超时
}

return 1; // 成功
}

// 5. DHT11 读取一个字节
uint8_t DHT11_Read(void)
{
uint8_t i, value = 0;
for (i = 0; i < 8; i++)
{
  while (!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)); // 等待位开始 (高)
  microDelay(40); // 延时40us
  if (!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) // 如果40us后还是低电平, 则是 0
  {
    value &= ~(1 << (7 - i));
  }
  else // 否则是 1
  {
    value |= (1 << (7 - i));
  }
  while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)); // 等待位结束 (低)
}
return value;
}

/* USER CODE END 0 */

/**
* @brief  The application entry point.
* @retval int
*/
int main(void)
{
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* MCU Configuration--------------------------------------------------------*/

/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
HAL_Init();

/* USER CODE BEGIN Init */

/* USER CODE END Init */

/* Configure the system clock */
SystemClock_Config();

/* USER CODE BEGIN SysInit */

/* USER CODE END SysInit */

/* Initialize all configured peripherals */
MX_GPIO_Init();
MX_I2C1_Init();
MX_TIM1_Init();
/* USER CODE BEGIN 2 */
HAL_TIM_Base_Start(&htim1); // 启动微秒定时器
ssd1306_Init(); // 初始化OLED

ssd1306_SetCursor(0, 0);
ssd1306_WriteString("Starting...", Font_7x10, White);
ssd1306_UpdateScreen();
HAL_Delay(1000);

//
// !!! --- 把变量定义在 while 循环外面 --- !!!
//
uint8_t RHI = 0, RHD = 0, TCI = 0, TCD = 0, SUM = 0;
uint8_t Temperature = 0; // 初始温度为0
uint8_t Humidity = 0;    // 初始湿度为0

// 字符串缓冲区
char temp_str[20];
char hum_str[20];
char fan_str[20]; // 用于调试风扇状态

/* USER CODE END 2 */

/* Infinite loop */
/* USER CODE BEGIN WHILE */
while (1)
{
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  // --- 1. 读取传感器 ---
  if (DHT11_Start())
  {
    RHI = DHT11_Read(); // 湿度整数
    RHD = DHT11_Read(); // 湿度小数
    TCI = DHT11_Read(); // 温度整数
    TCD = DHT11_Read(); // 温度小数
    SUM = DHT11_Read(); // 校验和

    // 只有在校验和正确时, 才更新温湿度值
    // 如果校验失败, Temperature 会保持上一次的正确数值
    if (RHI + RHD + TCI + TCD == SUM)
    {
      Temperature = TCI;
      Humidity = RHI;
    }
  }
  // (如果DHT11_Start()失败, Temperature 也会保持上一次的值)


  // --- 2. 控制逻辑 (风扇) ---
      // 硬件: 风扇在 PB4, 高电平有效 (SET = ON)
      // 逻辑: 温度 > 25 时 ON
      if (Temperature > 25)
      {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET); // <-- 控制 PB4, SET
        sprintf(fan_str, "FAN: ON");
      }
      else
      {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); // <-- 控制 PB4, RESET
        sprintf(fan_str, "FAN: OFF");
      }

      // --- 3. 控制逻辑 (蒸汽) ---
      // 硬件: 蒸汽在 PB10, 低电平有效 (RESET = ON)
      // 逻辑: 湿度 < 55 时 ON (你可以改回 45)
      if (Humidity < 55)
      {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET); // <-- 控制 PB10, RESET
      }
      else
      {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET); // <-- 控制 PB10, SET
      }


  // --- 4. 显示数据到OLED ---
  sprintf(temp_str, "Temp: %d C", Temperature);
  sprintf(hum_str, "Hum:  %d %%", Humidity);

  ssd1306_Fill(Black); // 清屏

  ssd1306_SetCursor(0, 5);  // 第1行
  ssd1306_WriteString(temp_str, Font_7x10, White);

  ssd1306_SetCursor(0, 20); // 第2行
  ssd1306_WriteString(hum_str, Font_7x10, White);

  ssd1306_SetCursor(0, 35); // 第3行 (调试行)
  ssd1306_WriteString(fan_str, Font_7x10, White);

  ssd1306_UpdateScreen(); // 推送数据到屏幕

  HAL_Delay(1000); // 每一秒钟重复一次

/* USER CODE END 3 */
} // <-- 这是 while(1) 的 }
} // <-- 这是 main() 的 }

/**
* @brief System Clock Configuration
* @retval None
*/
void SystemClock_Config(void)
{
RCC_OscInitTypeDef RCC_OscInitStruct = {0};
RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

/** Initializes the RCC Oscillators according to the specified parameters
* in the RCC_OscInitTypeDef structure.
*/
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
RCC_OscInitStruct.HSEState = RCC_HSE_ON;
RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
RCC_OscInitStruct.HSIState = RCC_HSI_ON;
RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
{
  Error_Handler();
}

/** Initializes the CPU, AHB and APB buses clocks
*/
RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                            |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
{
  Error_Handler();
}
}

/**
* @brief I2C1 Initialization Function
* @param None
* @retval None
*/
static void MX_I2C1_Init(void)
{

/* USER CODE BEGIN I2C1_Init 0 */

/* USER CODE END I2C1_Init 0 */

/* USER CODE BEGIN I2C1_Init 1 */

/* USER CODE END I2C1_Init 1 */
hi2c1.Instance = I2C1;
hi2c1.Init.ClockSpeed = 100000;
hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
hi2c1.Init.OwnAddress1 = 0;
hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
hi2c1.Init.OwnAddress2 = 0;
hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
if (HAL_I2C_Init(&hi2c1) != HAL_OK)
{
  Error_Handler();
}
/* USER CODE BEGIN I2C1_Init 2 */

/* USER CODE END I2C1_Init 2 */

}

/**
* @brief TIM1 Initialization Function
* @param None
* @retval None
*/
static void MX_TIM1_Init(void)
{

/* USER CODE BEGIN TIM1_Init 0 */

/* USER CODE END TIM1_Init 0 */

TIM_ClockConfigTypeDef sClockSourceConfig = {0};
TIM_MasterConfigTypeDef sMasterConfig = {0};

/* USER CODE BEGIN TIM1_Init 1 */

/* USER CODE END TIM1_Init 1 */
htim1.Instance = TIM1;
htim1.Init.Prescaler = 71;
htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
htim1.Init.Period = 65535;
htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
htim1.Init.RepetitionCounter = 0;
htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
{
  Error_Handler();
}
sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
{
  Error_Handler();
}
sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
{
  Error_Handler();
}
/* USER CODE BEGIN TIM1_Init 2 */

/* USER CODE END TIM1_Init 2 */

}

/**
* @brief GPIO Initialization Function
* @param None
* @retval None
*/
static void MX_GPIO_Init(void)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};

/* GPIO Ports Clock Enable */
__HAL_RCC_GPIOD_CLK_ENABLE();
__HAL_RCC_GPIOB_CLK_ENABLE();
__HAL_RCC_GPIOA_CLK_ENABLE();

/*Configure GPIO pin Output Level */
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_4, GPIO_PIN_RESET);

/*Configure GPIO pins : PB10 PB4 */
GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_4;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/*Configure GPIO pin : PB9 */
GPIO_InitStruct.Pin = GPIO_PIN_9;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = GPIO_NOPULL;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
* @brief  This function is executed in case of error occurrence.
* @retval None
*/
void Error_Handler(void)
{
/* USER CODE BEGIN Error_Handler_Debug */
/* User can add his own implementation to report the HAL error return state */
__disable_irq();
while (1)
{
}
/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
* @brief  Reports the name of the source file and the source line number
* where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t *file, uint32_t line)
{
/* USER CODE BEGIN 6 */
/* User can add his own implementation to report the file name and line number,
   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
